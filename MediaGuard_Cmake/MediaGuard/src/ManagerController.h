#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <atomic>
#include <chrono>
#include <thread>
#include <list>
#include <memory>
#include <string>
#include <errno.h> 
#include <signal.h>
#include "StreamManager.h"

class ManagerController
{
	typedef std::list<std::shared_ptr<StreamMangement>> StreamPtrList; //用於 函數 run_media_batch_list

private:
	StreamPtrList m_StreamPtrList; //用於 函數 run_media_batch_list 
public:
	explicit ManagerController();
	~ManagerController();

public:
	static std::string generate_directory(FileNameType type, std::string& video_path_str, std::string& audio_path_str, std::string& picture_path_str, std::string& hls_path_str);
	void Init();
	void Uninit();
	void run_media_list();
	void run_media_batch_list();

	ThreadPool m_ThreadPollForCtrl;
	 

public:
	static void clean_picture(int64_t picRemainMinutes);
	static void clean_video_store();
	static std::string current_working_directory(); 
	static double get_length(std::string filePath);
	static int64_t get_filesize(std::string path);

public:
	static void valid_op(int n);
	static void signal_check_main();
	static void main_initialize(); 
	static void http_server_start();
	static void create_main_media_folder();
};
