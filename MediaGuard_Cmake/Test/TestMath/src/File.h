#pragma once
#include <string>
#include <iostream>
#include <vector>
#include <filesystem>  // C++ std 17 
#include <chrono>
#include <rapidjson/document.h>
#include <rapidjson/filereadstream.h> 
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

#ifdef _WIN32

#include <windows.h>
#include <direct.h>
#include <io.h>

#define ST_access(file,mode) _access(file,mode)
#define ST_local_time(time,tm) localtime_s(&tm, &time)
#define ST_mkdir(dir) _mkdir(dir)
#define ST_remove(file) remove(file)
 

#else

#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>

#define ST_access(file,mode) access(file,mode)
#define ST_local_time(time,tm) localtime_r(&time, &tm)
#define ST_mkdir(dir) mkdir(dir, 775)
#define ST_remove(file) rm(file)
  
#endif

static const char kWindowSplash = '\\';
static const char kLinuxSplash = '/';
#ifdef _WIN32
static const char kSeprator = kWindowSplash;
#else
static const char kSeprator = kLinuxSplash;
#endif  

// #pragma once
//#include <string>
//#include <iostream>
//#include <vector>
//#include <iostream>
//#include <filesystem>  // C++ std 17 
//#include <chrono>
//
//#include <rapidjson/document.h>
//#include <rapidjson/filereadstream.h> 
//#include "rapidjson/stringbuffer.h"
//#include "rapidjson/writer.h"
//
//
//
//#ifdef _WIN32
//
//#include <windows.h>
//#include <direct.h>
//#include <io.h>
//
//#define ST_access(file,mode) _access(file,mode)
//#define ST_local_time(time,tm) localtime_s(&tm, &time)
//#define ST_mkdir(dir) _mkdir(dir)
//#define ST_remove(file) remove(file)
//
//#else
//
//#pragma once
//#include <unistd.h>
//#include <dirent.h>
//#include <sys/stat.h>
//#include <sys/types.h>
//
//#define ST_access(file,mode) access(file,mode)
//#define ST_local_time(time,tm) localtime_r(&time, &tm)
//#define ST_mkdir(dir) mkdir(dir, 775)
//#define ST_remove(file) rm(file)
//
//
//#endif
//
//
//static const char kWindowSplash = '\\';
//static const char kLinuxSplash = '/';
//#ifdef _WIN32
//static const char kSeprator = kWindowSplash;
//#else
//static const char kSeprator = kLinuxSplash;
//#endif // _WIN32

//=====================================================================
namespace fs = std::filesystem;

class File
{
	static const int kSuccess = 0;
	static const int kError = -1;
public:
	 
	/*獲取 應用程式根路徑*/
	static std::string GetWorkPath()
	{
#ifdef _WIN32 
		///current_working_directory
		char buff_workpath[255];
		_getcwd(buff_workpath, sizeof(buff_workpath));
		static std::string current_working_directory(buff_workpath);
		static std::string workpath = current_working_directory;
		return workpath;
#else 
		///current_working_directory  
		char buff_workpath[255];
		if (getcwd(buff_workpath, sizeof(buff_workpath)) != NULL) {
			std::cout << "File.h linux getcwd: " << buff_workpath << std::endl;
		}
		else {
			perror("File.h linux getcwd error");
		}
		static std::string current_working_directory(buff_workpath);
		static std::string workpath = current_working_directory;
		return workpath;
#endif //__linux__
	}

	/*
	* @brief: check the end of directory
	* @para[in]:    strDir, the directory for checking
	* @return:      bool, end with splash to return true, or false
	*/
	static bool CheckEndWithSplash(const std::string& strDir)
	{
		if ((strDir.back() != kWindowSplash)
			&& (strDir.back() != kLinuxSplash))
			return false;
		return true;
	}

	/*
	* @brief: create single directory
	* @data:  20200207
	* @update:
	* @para[in]:    strDir, the directory for creating
	* @return:      bool, success to return true, or false
	*/
	static bool CreateSingleDirectory(const std::string& strDir)
	{
		if (-1 == ST_access(strDir.c_str(), 0))
		{
			if (kSuccess == ST_mkdir(strDir.c_str()))
				return true;
			printf("Create directory:%s failed:", strDir.c_str());
			return false;
		}
		return true;
	}

	/*
	* @brief: create multi-directories
	* @data:  20200207
	* @update:
	* @para[in]:    strDir, the directory for creating
	* @return:      bool, success to return true, or false
	*/
	static bool CreateMultiDirectory(const std::string& strDir)
	{
		std::string strTmpPath(strDir);
		if (!CheckEndWithSplash(strTmpPath))
			strTmpPath += kSeprator;

		size_t nPrePos = 0;
		size_t nCurrPos = 0;
		while (nCurrPos = strTmpPath.find_first_of(kSeprator, nCurrPos), nCurrPos != -1)
		{
			std::string strTmpDir = strTmpPath.substr(0, ++nCurrPos);
			if (kSuccess != ST_access(strTmpDir.c_str(), 0))
			{
				if (kSuccess != ST_mkdir(strTmpDir.c_str()))
				{
					printf("Create directory:%s failed:", strDir.c_str());
					return false;
				}
			}
			nPrePos = nCurrPos;
		}
		return true;
	}

	/*
	* get_file_info win/linux
	*/  
	static bool get_file_info(const std::string& strFilePath, int& iCreateTime, int& iModifyTime, int& iAccessTime, int& iFileLen) {
		try {
			fs::path filepath(strFilePath);

			if (!fs::exists(filepath)) {
				std::cerr << "File::get_file_info File Not Exist : " << strFilePath << std::endl;
				return false;
			}

			auto fileStatus = fs::status(filepath);
			 
			iFileLen = static_cast<int>(fs::file_size(filepath)); 
			auto lstTime = fs::last_write_time(filepath); 
			iModifyTime = std::chrono::duration_cast<std::chrono::milliseconds>(lstTime.time_since_epoch()).count();    
			iAccessTime = iModifyTime;  //先應付需求,後續解決
			iCreateTime = iModifyTime;  //先應付需求,後續解決
			 
		}
		catch (const std::exception& e) {
			std::cerr << "Error while getting file info: " << e.what() << std::endl;
			return false;
		}

		return true;
	}
	/*
	* 刪除目錄 std::string directoryPath = "path/to/your/directory"; // 指定要刪除的目錄路徑
	* removeDirectory(directoryPath);
	*/ 
	static void removeDirectory(const std::string& path) {
		try {
			fs::path directoryPath(path);

			if (fs::exists(directoryPath)) {
				fs::remove_all(directoryPath);
				std::cout << "File::removeDirectory Directory removed: " << path << std::endl;
			}
			else {
				std::cerr << "File::removeDirectory Directory does not exist: " << path << std::endl;
			}
		}
		catch (const std::exception& e) {
			std::cerr << "File::removeDirectory Error while removing directory: " << e.what() << std::endl;
		}
	}

	/*
	* 判斷是否是一個文件夾
	*/
	static bool isDirectory(const std::string& path) {
		struct stat info;
		if (stat(path.c_str(), &info) != 0) {
			return false;
		}
		return (info.st_mode & S_IFDIR);
	}

	/*
	* 刪除文件函數
	*/
	static bool deleteFile(const std::string& filePathx) {

		std::filesystem::path filepath(filePathx);

		if (!std::filesystem::exists(filepath)) {
			std::cerr << "File::deleteFile File Not Exist : " << filepath.c_str() << std::endl;
			return false;
		}
		std::string strPath = filepath.string();
		const char* cfilepath = strPath.c_str();

		if (std::remove(cfilepath) != 0) {
			std::cerr << "File::deleteFile Failed to delete file: " << filepath.c_str() << std::endl;
			return false;
		}
		else {
			std::cout << "File::deleteFile File deleted successfully: " << filepath.c_str() << std::endl;
			return true;
		}
	}

	/*
	* @brief: get files of directory
	* @return:      bool, success to return true, or false
	*/
	static bool GetFilesOfDir(const std::string& strDir, std::vector<std::string>& vecFile) {
		try {
			for (const auto& entry : fs::directory_iterator(strDir)) {
				if (fs::is_regular_file(entry.path())) {
					vecFile.push_back(entry.path().string());
				}
			}
		}
		catch (const std::exception& e) {
			std::cerr << "Error while reading directory: " << e.what() << std::endl;
			return false;
		}

		return true;
	}
	/*
	*獲取文件下的子文件夾集合
	*/
	static void GetDirsOfDir(const std::string& path, std::vector<std::string>& dirs) {
		for (const auto& entry : std::filesystem::directory_iterator(path)) {
			if (entry.is_directory()) {
				dirs.push_back(entry.path().string());
			}
		}
	}
	 
	/*
	* support the json file reading
	* 讀取配置文件 deviceconfig.json
	*/
	static std::string readJsonFile(const std::string& strFile)
	{
		std::string jsonString = "";
		// 載入 JSON 檔案
		const char* filename = strFile.c_str();
		FILE* fp = fopen(filename, "rb");
		if (!fp) {
			std::cerr << "CAN NOT OPEN JSON FILE : " << filename << std::endl;
			return jsonString;
		}

		char readBuffer[65536];
		rapidjson::FileReadStream is(fp, readBuffer, sizeof(readBuffer));

		rapidjson::Document document;
		document.ParseStream(is);

		if (document.HasParseError()) {
			std::cerr << "PARSE  JSON FILE  ERROR OCCURED!!!" << strFile << std::endl;
			fclose(fp);
			return jsonString;
		}

		// 從 JSON 中讀取資料
		if (document.IsObject()) {
			// 將整個 JSON 文件轉換為字串
			rapidjson::StringBuffer buffer;
			rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
			document.Accept(writer);
			std::string jsonString = buffer.GetString();
			buffer.Flush();
			buffer.Clear();

			return jsonString;
		}
		else {
			fclose(fp);
			return jsonString;
		}
	}


	/*
	* 保存Json 文件
	* savePath = ./conf/abcdxxx.json
	*/
	static bool saveJsonFile(const std::string& strContent, const std::string& savePath) {

		std::string newSavePath = File::ParseLocalPath(savePath);

		using namespace rapidjson;
		Document document;
		document.Parse(strContent.c_str());

		if (document.HasParseError()) {
			std::cout << "func::saveJsonFile :: JSON parsing error! " << newSavePath;
			return false;
		}

		StringBuffer buffer;
		Writer<StringBuffer> writer(buffer);
		document.Accept(writer);

		std::ofstream ofs(newSavePath);

		if (!ofs.is_open()) {

			std::cout << "func::saveJsonFile :: Unable to open file! " << newSavePath;
			return  false;
		}

		ofs << buffer.GetString();
		ofs.close();

		return true; //"JSON 檔案已成功儲存"
	}

	/*
	* 轉換為本地操作系統路徑
	*/
	static std::string ParseLocalPath(const std::string& path_file_name)
	{
		try {
			namespace fs = std::filesystem;
			fs::path filePath(path_file_name);

			// 將路徑格式轉換為本地格式
			return filePath.generic_string();
		}
		catch (const std::exception& e) {
			// 錯誤處理：如果發生異常，返回原始路徑
			return path_file_name;
		}
	}

};