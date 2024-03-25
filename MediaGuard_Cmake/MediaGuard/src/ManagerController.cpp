#pragma once

#ifdef _WIN32
#include <io.h>
#endif
#include "Common.h"
#include "StreamDefine.h"
#include "ManagerController.h" 
#include "StreamHandle.h"
#include "StreamManager.h"
#include "./interface/CameraMpeg.h" 
#include "httpserver/comm.h"
#include "httpserver/httpserver.h"


using namespace Stream;

GlobalSetting g_GlobalSetting; //全局配置
// std::atomic需使用默认构造初始化, 不然有的编译器会因不支持拷贝构造导致编译不通过
std::atomic<bool> g_bInited(false);
std::atomic<bool> g_bExit(false);
std::atomic<bool> main_exit(false); //通过信号判断是否退出
std::mutex g_mtLock;
std::condition_variable g_cvCond;
std::thread g_thRun;
std::thread g_thPict;
std::thread g_thVideoStore;  

typedef std::list<StreamMangement> StreamMgtList;

typedef std::shared_ptr<StreamMangement> StreamHandlePtr;  //共享指針

ManagerController::ManagerController()
{ 
	//启动线程池
	m_ThreadPollForCtrl.Start();

	g_bExit = false;

	GlobalSetting globalSetting;  //这里加多一个函数获取json文件

	if (globalSetting.videoRemainMinutes < 15) // 规则保留最少15分钟的video文件
		globalSetting.videoRemainMinutes = 15;

	g_GlobalSetting = globalSetting;

	Init();
}

ManagerController::~ManagerController()
{
}

/* 每秒運行邏輯 是用於判斷 對時間比較敏感的目錄進行判斷*/
void run()
{   
	 
	main_exit = false;
	bool bFirstRun = true;
	 
	while (!g_bExit.load())
	{ 
		if (bFirstRun)  
		{ 
			fs::path video_path = fs::current_path() / kVideoDir / Time::GetCurrentDate();
			//fs::path audio_path = fs::current_path() / kAudioDir / Time::GetCurrentDate();
			 
			if (!File::isDirectoryExists(video_path.string()))
				File::CreateSingleDirectory(video_path.string());

			//audio文件夹暂时没用上，不启动创建
			/*if (!File::isDirectoryExists(audio_path.string()))
				File::CreateSingleDirectory(audio_path.string());*/
			  
			bFirstRun = false;
		}
		   
		SHARED_LOCK(g_mtLock);
		g_cvCond.wait_for(locker, std::chrono::milliseconds(1000), []() {
			auto isExit = g_bExit.load();
			//TEST
			//std::cout << Time::GetCurrentSystemTime() << "  g_bExit.load():" << isExit << std::endl;
			return isExit;
			});
	}
}

void clean_picture_run()
{
	while (!g_bExit.load())
	{
		ManagerController::clean_picture(g_GlobalSetting.picRemainMinutes);
		SHARED_LOCK(g_mtLock);
		g_cvCond.wait_for(locker, std::chrono::milliseconds(20000), []() {  //20's
			auto isExit = g_bExit.load();
			//TEST
			//std::cout << Time::GetCurrentSystemTime() << "  g_bExit.load():" << isExit << std::endl;
			return isExit;
			});
	}
}

void clean_video_store_run()
{
	while (!g_bExit.load())
	{
		ManagerController::clean_video_store();
		SHARED_LOCK(g_mtLock);
		g_cvCond.wait_for(locker, std::chrono::milliseconds(60000), []() {  //test 60's 实际设置长点，过于频繁不好，消耗资源
			auto isExit = g_bExit.load();
			//TEST
			//LOG(INFO) << "clean_video_store_run : 10's isExit=" << isExit << std::endl;
			return isExit;
			});
	}
}

void ManagerController::Init()
{
	if (!g_bInited.load()) {
		  
		g_thRun = std::thread(run);
		g_thPict = std::thread(clean_picture_run);
		g_thVideoStore = std::thread(clean_video_store_run);

		bool bInited = false;
		if (g_bInited.compare_exchange_strong(bInited, true, std::memory_order_acq_rel)) {
			// it needs do this while open dshow
			avformat_network_init();  //增加网络初始化
			avdevice_register_all();
			//显示 ffmpeg log记录等级
			if (DEVICE_CONFIG.cfgGlobalSetting.ffmpegOpenInfo)
				av_log_set_level(AV_LOG_INFO);
			else
				av_log_set_level(AV_LOG_ERROR);

		}

		// for clean hls index.m3u8 cache ts files 
		m_thread_hls_clear = std::thread(std::bind(&ManagerController::clean_hls_ts_run, this));
	}
}

/*
1、單個實例的方式，硬數據傳入  批處理啟動 測試用
*/
void ManagerController::run_media_list()
{
	//这里是把一个带有infoStream信息配置的rtsp列表 批处理启动StreamMangement对象
	//任意键停止也是通过列表批处理停止
	//现在是测试环境下 独立一个运行

	Stream::StreamInfo infoStream;
	infoStream.nCameraId = 12;
	infoStream.bSavePic = true;
	infoStream.mediaFormate = MediaFormate::FLV;
	infoStream.bSaveVideo = true;
	infoStream.savePictRate = 25;  //25%12 = 保存n张图片
	infoStream.nVideoTime = 60 * 1000 * 1; //60seconds*5  (record duration :5minutes)
	infoStream.nStreamDecodeType = (int)StreamDecodeType::HLS;
	// input
	//infoStream.strInput = "rtsp://admin:Admin123@192.168.2.64:554";
	infoStream.strInput = "video.mp4";

	std::cout << "Input Rtsp Url if need:" << "\n";
	std::string rtsp_url;
	getline(std::cin, rtsp_url);
	//std::cout << rtsp_url << std::endl;
	if (rtsp_url != "")
		infoStream.strInput = rtsp_url;

	// the camera in notebook
   // infoStream.strInput = "video=USB2.0 HD UVC WebCam";
	//infoStream.strInput = "video=USB2.0 PC CAMERA";
   // infoStream.strInput = "rtsp://root2:123456@192.168.10.90:554/axis-media/media.amp?videocodec=h264&resolution=640x480";

	// output
	//infoStream.strOutput = "test.mp4";
	infoStream.strOutput = "http://127.0.0.1:180/hls/0/index.m3u8";

	// hard device accelate
	//是否使用硬件加速，需要已知機器配置
	//infoStream.nHDType = Stream::kHWDeviceTypeNone;
	infoStream.nHDType = Stream::kHWDeviceTypeCUDA;
	//infoStream.nHDType = Stream::kHWDeviceTypeDXVA2;

	//RTSP方式 RTSP模式录像:  
	//LOG(INFO) << "input url :" << rtsp_url;
	int golPicRemainMinutes = 2;
	StreamMangement hStream(Stream::kStreamTypeRtsp);
	//StreamMangement hStream(Stream::kStreamTypeUsb);
	//StreamMangement hStream(Stream::kStreamTypeUsb); 
	hStream.ListDshowDevice();  //FOR USB CAM

	infoStream.nStreamDecodeType = (int)StreamDecodeType::HLS; //输出hls
	hStream.StartDecode(infoStream);

	ManagerController::signal_check_main(); //判断ctrl+c ctrl+b 提示exit 退出

	LOG(INFO) << "It'll exit in few seconds normally!";

	//任意键退出 前需要关闭其他线程  多个rtsp handle单元则for list退出
	hStream.StopDecode();

	std::this_thread::sleep_for(std::chrono::milliseconds(3000));

	std::cout << "It'll exit in few seconds." << "\n";

	system("pause");
}

/*
2、typedef std::list<std::shared_ptr<StreamMangement>>方式獲取實例指針  批處理啟動
*/
void ManagerController::run_media_batch_list()
{
	std::cout << "\nDEVICE_CONFIG::DEVICE SERIAL NUMBER : " << DEVICE_CONFIG.cfgDevice.device_serial_no << " | " << " Storage Limited bytes : " << to_string(DEVICE_CONFIG.cfgGlobalSetting.storageLimitedbytes) << "\n" << std::endl;
	if (DEVICE_CONFIG.cfgDevice.device_serial_no.length() == 0)
	{
		LOG(INFO) << "device.json uft8 unformat!!!(uft8 without bom) [System Pause]" << std::endl;;
		system("pause");
		return;
	}
	//獲取鏡頭列表
	CameraMpeg cameraMpeg;
	Service::StreamInfoApiList sList;
	bool get_list_succ = true;
	int i = 0;
	while (get_list_succ) {
		int nCode = cameraMpeg.camera_list(sList);
		if (nCode != CP_OK)
		{
			LOG(INFO) << "func::cameraMpeg.camera_list -> GET CAMERA LIST FROM CLOUD FAIL!!!";
		}
		else
			get_list_succ = false;

		if (i > 10)  //最多10次嘗試
		{
			LOG(INFO) << "func::cameraMpeg.camera_list -> TRY TO GET MORE THAN 10 Times, FAIL!!!";
			break;
		}
	}

	Service::StreamInList listx;
	bool ret_trans = cameraMpeg.camera_list_trans_to_strean_info(sList, listx);
	if (!ret_trans)
	{
		LOG(INFO) << "func::cameraMpeg.camera_list_trans_to_strean_info -> TRANS TO STREAM INFO FAIL!!!";
	}

	Service::StreamInList::iterator itr;
	for (itr = listx.begin(); itr != listx.end(); ++itr)
	{
		std::cout << "\nCamera Rtsp = " << itr->strInput << " ---------------------- \n" << std::endl;

		StreamInfo streamInfo;
		streamInfo.nCameraId = itr->nCameraId;
		streamInfo.rtspIp = itr->rtspIp;
		streamInfo.nHDType = itr->nHDType;
		streamInfo.strInput = itr->strInput;
		streamInfo.bSavePic = itr->bSavePic;
		streamInfo.mediaFormate = itr->mediaFormate;
		streamInfo.savePictRate = itr->savePictRate;
		streamInfo.bSaveVideo = itr->bSaveVideo;
		streamInfo.nVideoTime = itr->nVideoTime;
		streamInfo.nStreamDecodeType = itr->nStreamDecodeType;
		streamInfo.strOutput = itr->strOutput;
		streamInfo.nWidth = itr->nWidth;
		streamInfo.nHeight = itr->nHeight;
		streamInfo.nPixFmt = itr->nPixFmt;
		streamInfo.nFrameRate = itr->nFrameRate;
		streamInfo.nVideoIndex = itr->nVideoIndex;
		streamInfo.nAudioIndex = itr->nAudioIndex;
		streamInfo.nRefCount = itr->nRefCount;
		streamInfo.bRtmp = false;  //不開啟rtmp,

		std::shared_ptr<StreamMangement>  m_StreamPtrHandle = std::make_shared<StreamMangement>(Stream::kStreamTypeRtsp);
		m_StreamPtrHandle->ListDshowDevice();
		bool isStart = m_StreamPtrHandle->StartDecode(streamInfo);

		if (isStart)
		{
			m_StreamPtrList.push_back(m_StreamPtrHandle);
		}
	}

	//輸入exit則退出
	ManagerController::signal_check_main(); //判断ctrl+c ctrl+b 提示exit 退出

	LOG(INFO) << "It'll exit in few seconds normally!";

	//任意键退出 前需要关闭其他线程  多个rtsp handle单元则for list退出

	for (auto itr_mgt = m_StreamPtrList.begin(); itr_mgt != m_StreamPtrList.end(); ++itr_mgt)
	{
		(*itr_mgt)->StopDecode();  //auto x = (std::shared_ptr<StreamMangement>)(*itr_mgt);

		LOG(INFO) << "CameraId=" << (*itr_mgt)->m_infoStream.nCameraId << " PLEASE WAIT TO STOP IN TEN SECONDS, " << (*itr_mgt)->m_infoStream.rtspIp << std::endl;

		//例如 全局圖片List需要時間去處理。 關閉時間不足，導致圖片識別業務線程池未處理完畢導致變量關閉無法讀取
		std::this_thread::sleep_for(std::chrono::milliseconds(9000)); 

		LOG(INFO) << "CameraId=" << (*itr_mgt)->m_infoStream.nCameraId << " COMPLETED TO STOP CAMERA ID = " << (*itr_mgt)->m_infoStream.nCameraId << " CAMERA IP " << (*itr_mgt)->m_infoStream.rtspIp << std::endl;

	}
	
	m_StreamPtrList.clear();

	std::this_thread::sleep_for(std::chrono::milliseconds(3000));

	std::cout << "It'll exit in few seconds." << "\n";

	system("pause");
}

void ManagerController::Uninit()
{
	g_bExit.store(true);
	g_cvCond.notify_all();
	if (g_thRun.joinable())
		g_thRun.join();

	if (g_thPict.joinable())
		g_thPict.join();

	if (g_thVideoStore.joinable())
		g_thVideoStore.join();
}

/*
	获取piture 目录下的每个子目录 并判断目录下的图片是否已经过期
	picRemainMinutes 单位是分钟
*/
void ManagerController::clean_picture(int64_t picRemainMinutes)
{
	const fs::path picture_path = fs::current_path().append("picture"); //ManagerController::current_working_directory();
	//TEST  clean_picture
	//LOG(INFO) << "[TEST] [func::clean_picture] picture_path:" << picture_path << "\n";

	//tu-tu said:
	//c++17 跨平台 std::fs::current_path() 
	//如果编译器支持C++17，则建议使用std::filesystem::current_path
	//如果只在windows平台使用，可使用_getcwd
	//如果只在linux平台使用，可使用getcwd
	//如果代码要跨平台使用，可以使用通过编译统一的版本 
	//ref：https ://www.jianshu.com/p/2d9a5e0c7c48
	   
	std::vector<std::string> vecFile;
	File::GetFilesOfDir(picture_path.string(), vecFile);

	//TEST 清理过时图片，测试完毕 注释掉
	LOG(INFO) << "[FUNC::clean_picture] picRemainMinutes:" << picRemainMinutes << " vecFiles =" << vecFile.size()<<"\n";

	for (size_t i = 0; i < vecFile.size(); i++) {
		  
		int iCreateTime, iModifyTime, iAccessTime, iFileLen;

		const std::string picutre_path_filename = vecFile[i].c_str();

		if (true == File::get_file_info(vecFile[i].c_str(), iCreateTime, iModifyTime, iAccessTime, iFileLen))
		{
			//获取多少分钟前的时间
			std::chrono::system_clock::time_point curr = std::chrono::system_clock::now();
			std::chrono::system_clock::time_point before_minutes_time = curr - std::chrono::minutes(picRemainMinutes);
			auto longremaintime = std::chrono::duration_cast<std::chrono::microseconds>(before_minutes_time.time_since_epoch());
			int64_t longCreateTime = static_cast<int64_t>(iCreateTime);
			if (longCreateTime < longremaintime.count())
			{ 
				try {

					 fs::path picture_path_file(picutre_path_filename);
					 File::deleteFile(picutre_path_filename);
  
					 //LOG(INFO) << "func::clean_picture DeleteFile:" << picture_path_file.string();
					   
				}
				catch (...) {
					std::cout << Time::GetCurrentSystemTime() << picutre_path_filename << "[EXCEPTION] DELETED FAIL" << std::endl;
				}
			}
			//测试完毕注释掉2023-3-16
			else {
				LOG(INFO) <<"[delete picture] " << picutre_path_filename << "/t" << Time::GetCurrentSystemTime() << "[file not in " << picRemainMinutes << " minutes ago]" << " file create:" << longCreateTime << "-" << iCreateTime << " | " << longremaintime.count();
			}
		}
	}
}

/*
	如果指定限定存储容量，则执行清理存储的媒体文件，不包括图片，图片属于上面clean_picture函数的逻辑
	如果设置容量太小不适合此规则，例如100m，导致一个文件都容不下的情况下，录像中的文件可能会被卡住。或删除失败的情况
*/
void ManagerController::clean_video_store()
{
	if (g_GlobalSetting.storageLimitedbytes == 0) //不限制容量
		return;

	fs::path video_path = fs::current_path().append("video");
	 
	if (!fs::is_directory(video_path))
	{
		LOG(INFO) << "NO SUCH DIRECTORY [VIDEO] " << video_path << "\n" << std::endl;
		return;
	}

	std::vector<std::string> vecDires;
 
	File::GetDirsOfDir(video_path.string(), vecDires);

	//TEST
	//LOG(INFO) << "video_folder =" << video_folder << std::endl;

	int64_t storage_capacity = get_filesize(video_path.string());

	for (size_t i = 0; i < vecDires.size(); i++) {

		fs::path date_video_path = vecDires[i];
		  
		if (File::isDirectory(date_video_path.string()))
		{
			std::vector<std::string> vector_date_file;
			File::GetFilesOfDir(date_video_path.string(), vector_date_file);

			for (size_t j = 0; j < vector_date_file.size(); j++)
			{
				if (storage_capacity > g_GlobalSetting.storageLimitedbytes)
				{ 
					int iCreateTime, iModifyTime, iAccessTime, iFileLen;
					if (true ==File:: get_file_info(vector_date_file[j].c_str(), iCreateTime, iModifyTime, iAccessTime, iFileLen))
					{
						//规则保留最少15分钟的文件
						//获取多少分钟前的时间
						std::chrono::system_clock::time_point curr = std::chrono::system_clock::now();
						std::chrono::system_clock::time_point before_minutes_time = curr - std::chrono::minutes(g_GlobalSetting.videoRemainMinutes);
						auto longremaintime = std::chrono::duration_cast<std::chrono::seconds>(before_minutes_time.time_since_epoch());
						int64_t longCreateTime = static_cast<int64_t>(iCreateTime);
						if (longCreateTime < longremaintime.count())
						{
							fs::path video_path_file(vector_date_file[j]);
							bool del_res = File::deleteFile(video_path_file.string());
						}

						//删除后再次检查是否满足限制容量的要求，如果满足则退出删除检查循环
						storage_capacity = get_filesize(video_path.string());
						if (storage_capacity < g_GlobalSetting.storageLimitedbytes)
						{
							//LOG(INFO) << storage_capacity << g_GlobalSetting.storageLimitedbytes << " " << video_folder << std::endl;
							break;
						}
					}
					else {
						//无法获取文件信息 File::get_file_info(...)
					}
				}
				storage_capacity = get_filesize(video_path.string());

				//TEST
				//LOG(INFO) << "video folders compacity limited storage_capacity =" << storage_capacity << " vector_date_file count =" << vector_date_file.size() << std::endl;
			}
		}
  
		//最后判断非当天的文件夹是否为NULL 规则：没有文件的文件夹删除
		std::vector<std::string> vec_check_blank_date_file;
		File::GetFilesOfDir(vecDires[i].c_str(), vec_check_blank_date_file);
		int64_t date_folder_files_count = vec_check_blank_date_file.size();
		const std::string todate = Time::GetCurrentDate();
		if (date_folder_files_count == 0)
		{
			if (vecDires[i].find(todate) == vecDires[i].npos)
			{
				fs::remove_all(vecDires[i]); 
				//TEST
				//LOG(INFO) << "Delete Date Folder Result: " << date_folder << "del_date_folder_res =" << del_date_folder_res << std::endl;
			}
		}

		storage_capacity = get_filesize(video_path.string()); //已经在子循环内进行检查
		if (storage_capacity < g_GlobalSetting.storageLimitedbytes)
		{
			break;
		}
	}
}

std::string ManagerController::current_working_directory()
{ 
	return fs::current_path().string();
//#ifdef _WIN32
//	char buff[MAX_PATH];
//	_getcwd(buff, MAX_PATH);
//	std::string current_working_directory(buff);
//	return current_working_directory;
//#elif __linux__
//	
//#include <unistd.h>
//#include <limits.h> 
//
//	char buff[PATH_MAX];
//	if (getcwd(buff, PATH_MAX) != NULL) {
//		std::string current_working_directory(buff);
//		return current_working_directory;
//	}
//	else {
//		return "";  // 或者返回其他錯誤處理
//	}
//	 
//#endif
}
 
/*
https ://blog.csdn.net/weixin_47826078/article/details/117751305
cout<<"test文件夹的大小："<< get_filesize(path)<<"bytes"<<endl;
获取文件大小的函数，返回值以KB为单位
*/
double ManagerController::get_length(string filePath) {
	double size_byte = 0;

	ifstream fin(filePath);
	if (fin.is_open())
	{
		fin.seekg(0, ios::end);
		double size = fin.tellg();
		if (size == 0) {
			size_byte = 0;
		}
		else {
			size_byte = size;
		}
		fin.close();
	}
	return size_byte;
}
/*
* 用法
* string path("C:\\Users\\SeanWayen\\Desktop\\test");
* cout<<"test文件夹的大小："<< get_filesize(path)<<"bytes"<<endl;
*/
int64_t ManagerController::get_filesize(string path)//递归核心代码
{
#ifdef _WIN32
	//文件句柄
	intptr_t hFile = 0;
	vector<string> files;
	//文件信息
	struct _finddata_t fileinfo;
	string p;
	double filesize = 0;
	int i = 0;
	if ((hFile = _findfirst(p.assign(path).append("\\*").c_str(), &fileinfo)) != -1)
	{
		do
		{
			if ((fileinfo.attrib & _A_SUBDIR))//判断是否是文件夹 
			{
				//如果是目录,递归查找并累积size
				if (strcmp(fileinfo.name, ".") != 0 && strcmp(fileinfo.name, "..") != 0) {
					filesize += ManagerController::get_filesize(p.assign(path).append("\\").append(fileinfo.name));
				}
			}
			else
			{
				//如果不是，直接累加size
				filesize += ManagerController::get_length(path + "\\" + fileinfo.name);
			}
		} while (_findnext(hFile, &fileinfo) == 0);
		_findclose(hFile);
	}
	int64_t size_byte_int64 = static_cast<int64_t>(round(filesize)); // round(filesize);

	return size_byte_int64;
#elif __linux__
	struct stat fileStat;

	if (stat(path.c_str(), &fileStat) == 0) {
		return fileStat.st_size;
	}
	else {
		return -1; // 獲取文件大小失敗
	}
	return 0;
#endif
}


void ManagerController::valid_op(int n)
{
	if (n == SIGINT)
	{
		std::cout << "\n按下 ctrl+c 無效！請輸入 exit" << std::endl;
		signal(SIGINT, valid_op);
		main_exit = false;
	}
	if (n == SIGABRT)
	{
		std::cout << "\nSIGABRT：因中止呼叫而觸發的異常終止\n";
		main_exit = false;
	}
}


/*
* 判斷 ctrl+c ctrl+break 等等終斷操作的 拒絕操作提示返回.
*/
void ManagerController::signal_check_main()
{
	std::string input;

	while (!main_exit.load())
	{
		if (input != "")
		{
			transform(input.begin(), input.end(), input.begin(), ::tolower);
		}
		else {
			std::cout << "\nInput exit again to treminate!\n\n" << std::endl;
		}
 
		std::this_thread::sleep_for(std::chrono::milliseconds(100));

		if (std::cin.fail())
		{
			std::cout << "\nInput exit again to treminate!\n" << std::endl;
			std::cin.clear();
			std::cin.sync();
		}
		if (input == "exit")
		{
			LOG(INFO) << "\nnow will be exit as you input = " << input << "\n";
			main_exit = true;
			break;
		}
		else if (input != "") {

			std::cout << "\nyou input :(" << input << ") invalid,input exit to end.\n" << std::endl;
		}

		getline(std::cin, input);

		signal(SIGINT, valid_op); // /* 處理 SIGINT 信號 注册ctrl+c信号捕获函数*/
		signal(SIGABRT, valid_op); // 處理 SIGABRT 信號 | SIGABRT : abnormal termination triggered by abort call 
		  
	};
}

void ManagerController::main_initialize()
{
#ifdef _WIN32
	SetConsoleTitle("\nMedia Guard Ver3.0 ");
#elif __linux__
	std::cout << "\nMedia Guard Ver3.0 \n\n\n" << std::endl;
  
#endif

	std::string main_root = current_working_directory();
	std::cout << "\n\tCurrent working directory:" << main_root << "\n" << std::endl;

	ManagerController::create_main_media_folder();
}


/// <summary>
/// 啟動 HttpServer
/// </summary>
void ManagerController::http_server_start()
{
	httpserver::http_server_run();
	return;
}

/* 
* 創建主要的文件夾 video ; hls ; picture ; 
* 必須放在 ManagerController::http_server_start http服務啟動之前
*/
void ManagerController::create_main_media_folder()
{
	fs::path video_path = fs::current_path() / kVideoDir;
	fs::path picture_path = fs::current_path() / kPictureDir;
	fs::path hls_path = fs::current_path() / kHlsDir;

	if (!File::isDirectoryExists(video_path.string()))
		File::CreateSingleDirectory(video_path.string());

	if (!File::isDirectoryExists(picture_path.string()))
		File::CreateSingleDirectory(picture_path.string());

	if (!File::isDirectoryExists(hls_path.string()))
		File::CreateSingleDirectory(hls_path.string());
}


/* testing for return a .ts file list from index.m3u8
 * Target :  to  clean the .ts file exclude the index.m3u8 ts list files
*/
bool ManagerController::get_ts_list_file_from_index_m3u8(const std::string& path, std::vector<std::string>& vectorfiles)
{
	if (fs::exists(path))
	{
		std::ifstream file(path);

		std::string line;

		if (file.is_open()) {

			while (std::getline(file, line)) {

				if (!line.empty()) {
					// Remove trailing spaces
					while (std::isspace(line.back())) {
						line.pop_back();
					}

					if (line != "index.m3u8" && line[0] != '#' && line.substr(line.size() - 4) != ".tmp") {

						fs::path path(line);
						if (path.extension() == ".ts") {
							vectorfiles.push_back(line);
							//TEST LOG
							std::cout << "Add a file vectorfiles " << line << std::endl;
						}
						else {
							vectorfiles.push_back(line);
							//TEST LOG
							std::cout << "no .ts file contained,discard to add to ts file list" << line << std::endl;
						}
					}
				}
			}
			file.close();
			return true;
		} 
	}
	else {
		//no index.m3u8
		std::cout << "no index.m3u8 file in app root" << std::endl;
	}

	// no any ts list or no index.m3u8 files
	return false;
}


void ManagerController::delete_ts_list_file(const std::string& hls_cmaera_id_folder, std::vector<std::string>& vector_ts_files_index_m3u8)
{
	if (fs::is_directory(hls_cmaera_id_folder))
	{
		std::vector<std::string> vector_ts_files;
		if (File::GetFilesOfDir(hls_cmaera_id_folder, vector_ts_files) == true)
		{
			if (vector_ts_files.size() > 0)
			{
				for (std::vector<std::string>::iterator it = vector_ts_files.begin(); it != vector_ts_files.end(); ++it) {

					std::cout << *it << std::endl;

					if (std::find(vector_ts_files_index_m3u8.begin(), vector_ts_files_index_m3u8.end(), *it) == vector_ts_files_index_m3u8.end()) {
						
						File::deleteFile(*it);
					}
					else {
						++it;
					}
				}
				vector_ts_files.clear(); 
			} 
		}  
	} 
	return ;
}

 
void ManagerController::clean_each_hls_camera_folder()
{ 
	CameraMpeg cameraMpeg; 
	Service::StreamInfoApiList sList;
	int nCode = cameraMpeg.camera_list(sList);
	if (nCode != CP_OK)
	{
		LOG(INFO) << "func::ManagerController.clean_each_hls_camera_folder -> GET CAMERA LIST FROM CLOUD FAIL!!!";
	}
	Service::StreamInfoApiList::iterator itr;
	for (itr = sList.begin(); itr != sList.end(); ++itr)
	{ 
		 int camera_id = itr->nCameraId;
		 const fs::path hls_camera_folder = fs::current_path() / kHlsDir / std::to_string(camera_id);
		 std::vector<std::string> vector_ts_files_index_m3u8 ;
		 bool ret = get_ts_list_file_from_index_m3u8(hls_camera_folder.string(),vector_ts_files_index_m3u8);
		 if (ret == true)
		 {
			 delete_ts_list_file(hls_camera_folder.string(), vector_ts_files_index_m3u8);
		 }
		 vector_ts_files_index_m3u8.clear();
	} 
}


void ManagerController::clean_hls_ts_run()
{ 
	while (!m_bExit.load())
	{
		//TEST
		LOG(INFO) << "func::ManagerController.clean_hls_ts_run -> TESTING!!!";
		clean_each_hls_camera_folder();
		SHARED_LOCK(m_mtLock);  // std::unique_lock<std::mutex> locker(lock);
		m_cvCond.wait_for(locker, std::chrono::milliseconds(60000), [this]() {  //60's
			auto isExit = m_bExit.load();
			return isExit;
		});
	}
}