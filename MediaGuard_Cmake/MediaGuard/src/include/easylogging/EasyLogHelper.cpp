#pragma once
#include "EasyLogHelper.h" 
#include <iostream>
#include <filesystem>

using namespace Config;

EasyLogHelper::EasyLogHelper()
{
}

EasyLogHelper::~EasyLogHelper()
{
}

/*
	https://github.com/amrayn/easyloggingpp 下载
*/
bool EasyLogHelper::InitConLog()
{
	try
	{
		el::Configurations conf("log.conf");
		el::Loggers::reconfigureAllLoggers(conf);

	}
	catch (const std::exception& e)
	{
		std::cout << "Init easylogging++ error:" << e.what();
		return false;
	}
	return true;
}

bool EasyLogHelper::InitLogging()
{ 
	//ref: https://blog.csdn.net/samenmoer/article/details/112425060
	//ref: https://blog.csdn.net/w1820020635/article/details/115566364
	//ref:https://blog.csdn.net/zqy_china_world/article/details/128229124
	try
	{
		el::Configurations conf;

		//设置日志文件目录以及文件名
		conf.setGlobally(el::ConfigurationType::Filename, "log/log_%datetime{%Y%M%d}.log");
		// 启用日志
		conf.setGlobally(el::ConfigurationType::Enabled, "true");
		//设置日志文件最大文件大小
		conf.setGlobally(el::ConfigurationType::MaxLogFileSize, "2000000"); //20,971520 =20M

		//是否写入文件
		conf.setGlobally(el::ConfigurationType::ToFile, "true");
		//是否输出控制台
		conf.setGlobally(el::ConfigurationType::ToStandardOutput, "true");
		//设置日志输出格式
		//conf.setGlobally(el::ConfigurationType::Format, "[%datetime] [%loc] [%level] : %msg");  //定位到具体cpp文件的debug
		conf.setGlobally(el::ConfigurationType::Format, "[%datetime{%Y-%M-%d %H:%m:%s %z}] [%level] : %msg");
		//设置日志文件写入周期，如下每100条刷新到输出流中 
		//conf.setGlobally(el::ConfigurationType::LogFlushThreshold, "1");

		//设置配置文件
		el::Loggers::reconfigureAllLoggers(conf);
	}
	catch (const std::exception& e)
	{
		std::cout << "Init easylogging++ error:" << e.what();
		return false;
	}
	return true;
}


