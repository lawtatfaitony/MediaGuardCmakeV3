#pragma once
#include <string>
#include <chrono>
#include <ctime> 
#include <iomanip>
#include <iostream>
#include <sstream> 
#ifdef __linux__
#include <time.h>
#endif

class Time
{
public:
	static std::string GetCurrentDate()
	{
		time_t ltime = time(NULL);
		std::tm ltm = { 0 };
#ifdef _WIN32
		localtime_s(&ltm, &ltime);
#elif __linux__
		// 请自行百度使用localtime_r代替
		//localtime_r();
#endif
		char buffer[128] = { 0 };

		strftime(buffer, sizeof(buffer), "%Y-%m-%d", &ltm);
		return buffer;
	}

	static std::string GetCurrentSystemTime()
	{
		time_t ltime = time(NULL);
		std::tm ltm = { 0 };
#ifdef _WIN32
		localtime_s(&ltm, &ltime);
#elif __linux__
		// 请自行百度使用localtime_r代替
		//localtime_r();
#endif
		char buffer[128] = { 0 };
		strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &ltm);
		return buffer;
	}

	static int64_t GetMilliTimestamp()
	{
		auto curr = std::chrono::system_clock::now();
		auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
			curr.time_since_epoch());
		return ms.count();
	}

	static int64_t GetTimestamp()
	{
		auto curr = std::chrono::system_clock::now();
		auto ms = std::chrono::duration_cast<std::chrono::seconds>(
			curr.time_since_epoch());
		return ms.count();
	}
};

