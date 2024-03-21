#include "ManagerController.h"

//TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST
/* 每秒運行邏輯 是用於判斷 對時間比較敏感的目錄進行判斷*/

ManagerController::ManagerController()
	:kVideoDir("video")
	,kPictureDir("picture")
	,kAudioDir("audio")
	,kHlsDir("hls") 
	,g_bInited(false)
	,g_bExit(false)
	,main_exit(false) //通过信号判断是否退出 
{
	std::cout << "\nManagerController::ManagerController Hello World\n";
}


void ManagerController::run()
{
	bool bFirstRun = true; 

	while (!g_bExit.load())
	{
		if (bFirstRun)
		{
			fs::path video_path = fs::current_path() / kVideoDir / Time::GetCurrentDate();
			//fs::path audio_path = fs::current_path() / kAudioDir / Time::GetCurrentDate();

			if (!File::isDirectoryExists(video_path.string())) {

				File::CreateSingleDirectory(video_path.string());
				// TEST 
				std::cout << Time::GetCurrentSystemTime() << "  File::isDirectoryExists [false]:" << video_path.string() << std::endl;
			}
			else {
				// TEST 
				std::cout << Time::GetCurrentSystemTime() << "  File::isDirectoryExists [true]:" << video_path.string() << std::endl;
			}

			//g_bExit.load()
			std::cout << Time::GetCurrentSystemTime() << " g_bExit.load():" << g_bExit.load() << std::endl;

			//audio文件????用上，不???建
			/*if (!File::isDirectoryExists(audio_path.string()))
				File::CreateSingleDirectory(audio_path.string());*/

			bFirstRun = false;
		}

		SHARED_LOCK(g_mtLock);

		g_cvCond.wait_for(locker, std::chrono::milliseconds(1000), []() {
			//auto isExit = g_bExit.load();
			//TEST
			std::cout << Time::GetCurrentSystemTime() << "  g_bExit.load():" << "isExit" << std::endl;
			return false; // g_bExit.load();
			});
	}

	char input_char;
	std::cin.get(input_char);
	if (input_char == char('e'))
		g_bExit = true;

}