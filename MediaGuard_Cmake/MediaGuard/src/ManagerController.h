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

	std::thread m_thread_hls_clear;

	std::atomic<bool> m_bExit/* = false*/;//bool m_bExit;
	std::mutex m_mtLock;
	std::condition_variable m_cvCond;

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
	bool get_ts_list_file_from_index_m3u8(const std::string& path, std::vector<std::string>& vectorfiles);

	// delete  ts files from hls_cmaera_id_folder exclude in index.m3u8 file
	void delete_ts_list_file(const std::string& path, std::vector<std::string>& vectorfiles);

	// according to the [StreamPtrList] foreach to remove each camer hls folder'ts list file
	void clean_each_hls_camera_folder();

	// timer for cleaning up the ts file of hls 定時清理hls 的 ts 文件 在 line:140 applied
	void clean_hls_ts_run();
};
