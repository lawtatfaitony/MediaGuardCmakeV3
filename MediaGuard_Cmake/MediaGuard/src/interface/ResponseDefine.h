#pragma once
#include <string>
#ifdef _WIN32
#include <Time.h> 
#elif __linux__
// 请自己补linux内容
#endif
#include <sstream> 
namespace Service
{
	// 响应结构 
	struct Meta
	{
		bool success;
		std::string message;
		int errorCode;
	};
}
