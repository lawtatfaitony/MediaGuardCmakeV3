#pragma once
#include "Common.h" //全局包含文件
#include "StreamManager.h"
#include "RtspStreamHandle.h" 
#include "ManagerController.h" 
#include "StreamManager.h"
//test
#include "./interface/CameraMpeg.h" //CameraMpeg所有的CLOUD API基本放這裡
//#define ELPP_STL_LOGGING
//#define ELPP_THREAD_SAFE

INITIALIZE_EASYLOGGINGPP

using namespace Stream;

std::thread m_http_server;

int main(int argc, char** argv)
{
	Config::EasyLogHelper::InitLogging();
	LOG(INFO) << "Hello World...................";
	 
	LOG(INFO) << "Console Platform Is Required UTF8 Encoding.";
	

	//test============================================
	/*char c;
	std::cin.get(c);
	std::cout << "your input1 is：" << c << std::endl;
	char d;
	std::cin.get(d);
	std::cout << "your input2 is：" << d << std::endl;*/

	//測試車牌配置節點
	/*auto permitedCamList = DEVICE_CONFIG.cfgCarPlateRecogBusiness.PermitedCamList;
	for (auto item = permitedCamList.begin(); item != permitedCamList.end(); ++item)
	{
		std::cout << "permitedCamList ..................." << item->CameraId << std::endl;
	}*/

	//================================================================================================  
	ManagerController::main_initialize();

	m_http_server = std::thread(std::bind(&ManagerController::http_server_start));

	//------------------------------------------------------------------
	ManagerController managerController;

	//managerController.run_media_list();  //硬數據測試
	managerController.run_media_batch_list(); //多路camera 啟動處理 

	//任意键退出 前需要关闭其他线程
	managerController.Uninit();

	std::this_thread::sleep_for(std::chrono::milliseconds(3000));

	system("pause");

	//close the http server thread
	if (m_http_server.joinable())
		m_http_server.join();

	return 0;
}
