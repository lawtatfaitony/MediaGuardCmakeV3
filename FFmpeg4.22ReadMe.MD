# FFMPEG 编译库下载地址：
https://ffmpeg.zeranoe.com/builds/

# 版本4.22  

#### ffmpeg version 4.2.2 Copyright (c) 2000-2019 the FFmpeg developers
built with gcc 9.2.1 (GCC) 20200122

	路径 \\MediaGuard\include\ffmpeg
	
	　　FFmpeg是一套可以用來記錄、轉換數字音頻、視頻，并能将其轉化爲流的開源計算機程序。采用LGPL或GPL許可證。它提供了錄制、轉換以及流化音視頻的完整解決方案。它包含了非常先進的音頻/視頻編解碼庫libavcodec，FFmpeg在Linux平台下開發，但它同樣也可以在其它操作系統環境中編譯運行，包括Windows、Mac OS X等。

# FFmpeg 4.2.2正式版發布 更新亮點介紹（附下載地址）

­# FFmpeg 4.2.2 已正式發布，代号 “Wu” ，更新亮點包括：

# MagicYUV 編解碼器（實驗性）

­　　Bitstream 過濾器，用于在 H.264，HEVC 和 MPEG-2 流中編輯元數據

­　　TiVo ty/ty demuxer

­　　原生 aptX 和 aptX HD 編解碼器

­　　英特爾 QSV 加速 MJPEG 編碼

­　　英偉達 NVDEC 加速 H.264，HEVC，MJPEG，MPEG-1/2/4，VC1，VP8 / 9 hwaccel 解碼

­　　英特爾 QSV 加速 overlay filter

­　　mcompand audio filter

­　　acontrast audio filter

­　　OpenCL overlay filter

­　　video mix filter

­　　video normalize filter

­　　完整更新内容請查閱發行說明。

­　　FFmpeg 是用于處理音頻、視頻、字幕和相關元數據的多媒體内容的庫和工具的合集。

­　　庫：

­　　libavcodec 提供廣泛的編解碼器實現

­　　libavformat 流協議、容器格式和基本 I / O 訪問實現

­　　libavutil hashers、decompressors 和 miscellaneous utility functions 實現

­　　libavfilter 解碼的音頻和視頻過濾修飾實現

­　　libavdevice 訪問捕獲和播放設備抽象化實現

­　　libswresample 音頻混合和程序重采樣實現

­　　libswscale 光色轉換和程序縮放實現

­　　工具：

­　　ffmpeg 操縱、轉換和流傳輸多媒體内容的命令行工具箱

­　　ffplay 簡約的多媒體播放器

­　　ffprobe 檢查多媒體内容的簡單分析工具

­　　ffserver 直播多媒體流服務器

­　　附加的一些小工具，比如 aviocat、 ismindex 和 qt-faststart

­　　下載地址：

­　　http://ffmpeg.org/download.html



在直播過程中，如果想要清理掉不被 HLS 播放列表（例如 index.m3u8）所包含的文件，您可以通過以下步驟來實現：

1. **定期檢查和清理**：
   - 建立一個定時任務或背景執行緒，定期檢查目錄中的文件列表。
   - 對比 HLS 播放列表中的文件與實際存在的文件，刪除未被索引的文件。

2. **實現清理功能**：
   - 獲取 HLS 播放列表（index.m3u8）中列出的所有文件名。
   - 遍歷目錄，對比 HLS 播放列表中的文件名，刪除未被索引的文件。

3. **示例程式碼**：
   - 下面是一個簡單的示例程式碼，用於清理 HLS 播放列表中未被索引的文件：

```cpp
#include <iostream>
#include <fstream>
#include <vector>
#include <string> 

namespace fs = std::experimental::filesystem;

void clean_hls_unused_files(const std::vector<std::string>& indexed_files, const std::string& directory_path) {
    for (const auto& entry : fs::directory_iterator(directory_path)) {
        if (fs::is_regular_file(entry)) {
            std::string filename = entry.path().filename().string();
            if (std::find(indexed_files.begin(), indexed_files.end(), filename) == indexed_files.end()) {
                fs::remove(entry.path());
                std::cout << "Removed file: " << filename << std::endl;
            }
        }
    }
}

int main() {
    std::vector<std::string> indexed_files = {"file1.ts", "file2.ts", "file3.ts"}; // 從 index.m3u8 中獲取的已索引文件列表
    std::string directory_path = "your_directory_path"; // HLS 文件目錄路徑

    clean_hls_unused_files(indexed_files, directory_path);

    return 0;
}


/*來自文件 RtspStreamHandle.cpp 這段作廢*/
void RtspStreamHandle::clean_hls_ts_run_deprecated()
{
	if (m_infoStream.nStreamDecodeType != StreamDecodeType::HLS)
	{
		//必须是HLS流请求才继续执行ts切片清理
		return;
	}
	while (!m_bExit.load())
	{
		clean_hls_ts(30); //保留20秒的文件记录 注意设置hls av_dist的时候不能大于这个时间，
		SHARED_LOCK(m_mtLock);
		m_cvCond.wait_for(locker, std::chrono::milliseconds(60000), [this]() {  //60's
			auto isExit = m_bExit.load();
			return isExit;
			});
	}
}

void RtspStreamHandle::clean_hls_ts(int tsRemainSeconds)
{
	std::string camera_path = kHlsDir + "/" + std::to_string(m_infoStream.nCameraId) + "/";
	std::cerr << "RtspStreamHandle::clean_hls_ts:camera_path = "<< camera_path << std::endl;

	std::string hlsIndexm3u8 = camera_path + "/index.m3u8";
	if (-1 == ST_access(hlsIndexm3u8.c_str(), 0)) //-1=不存在,则创建 子文件夹（camerId）
	{
		int ret = ST_mkdir(camera_path.c_str());  //若目录创建成功，则返回0；否则返回-1
		if (ret != 0)
			LOG(ERROR) << std::to_string(ret) << " - CREATE FOLDER OF CAMERA HLS FAIL:" << camera_path.c_str() << "\n";
	}
	std::vector<std::string> vecFile;
	File::GetFilesOfDir(camera_path, vecFile);

	for (size_t i = 0; i < vecFile.size(); i++) {

		std::ostringstream ss;
		ss.exceptions(std::ostringstream::badbit);
		try {
			ss << camera_path << vecFile[i];
		}
		catch (const std::ios_base::failure& ex) {
			std::cerr << "Caught: std::ios_base::failure (func::clean_hls_ts)" << std::endl;
		}
		catch (const std::bad_alloc& ex) {
			std::cerr << "Caught: std::bad_alloc(func::clean_hls_ts)" << std::endl;
		}
		catch (...) {
			std::cerr << "Caught: ellipsis(func::clean_hls_ts)" << std::endl;
		}

		std::string ts_pathFilename = ss.str();
		ss.clear();

		int iCreateTime, iModifyTime, iAccessTime, iFileLen;

		if (true == File::get_file_info(ts_pathFilename, iCreateTime, iModifyTime, iAccessTime, iFileLen))
		{
			//获取多少分钟前的时间
			std::chrono::system_clock::time_point curr = std::chrono::system_clock::now();
			std::chrono::system_clock::time_point before_minutes_time = curr - std::chrono::seconds(tsRemainSeconds);
			auto longremaintime = std::chrono::duration_cast<std::chrono::seconds>(before_minutes_time.time_since_epoch());
			int64_t longCreateTime = static_cast<int64_t>(iCreateTime);
			if (longCreateTime < longremaintime.count())
			{
				const char* pfile = ts_pathFilename.c_str();
				try {
					 
					File::deleteFile(pfile);

					////删除后，尝试访问，如果还存在 则删除不成功。由于过于频繁影响速度，屏蔽掉
					//if (-1 != ST_access(pfile, 0)) { 
					//	LOG(ERROR) << Time::GetCurrentSystemTime() << ts_pathFilename << " DELETED FAIL:" << pfile << "file create:" << longCreateTime << "-" << iCreateTime << " | " << longremaintime.count() << std::endl;
					//}

				}
				catch (...) {
					LOG(ERROR) << ts_pathFilename << " [EXCEPTION] DELETED TS FILE FAIL" << std::endl;
				}
			}
			//for TEST
			/*else {
				LOG(INFO) << Time::GetCurrentSystemTime() << "[file not in " << tsRemainSeconds << " seconds ago]" << " file create:" << longCreateTime << "-" << iCreateTime << " | " << longremaintime.count();
			}*/
		}
	}
}
```

請將這段程式碼嵌入到您的應用程式中，並根據您的實際情況調整目錄路徑和已索引文件列表。這樣可以幫助您實現清理不被 HLS 播放列表包含的文件的功能。

 