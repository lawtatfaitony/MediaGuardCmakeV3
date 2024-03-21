#pragma once
#include <sstream> 
#include <chrono>
#include <cstdio>
#include "httplib/httplib.h"
#include "../File.h"

#include <iostream>
#include <stdlib.h>

#ifdef _WIN32
#include <direct.h>
#include <io.h>
#else

#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#endif // _WIN32

#define SERVER_CERT_FILE "./cert.pem"；
#define SERVER_PRIVATE_KEY_FILE "./key.pem"；

using namespace std;

class httpserver
{

public:
	explicit httpserver();
	~httpserver();

	//static void allowCorsAccess(httplib::Response& res);

	//static  std::string dump_headers(const Headers& headers);

	//static  std::string http_server_log(const Request& req, const Response& res);

	//static  bool UserPasswordValidByToken(const httplib::Request& req, httplib::Response& res);

	static int http_server_run(void);

#ifdef _WIN32
	static string GbkToUtf8(const char* src_str);
	static string Utf8ToGbk(const char* src_str);
#elif __linux__
	int GbkToUtf8(char* str_str, size_t src_len, char* dst_str, size_t dst_len);
	int Utf8ToGbk(char* src_str, size_t src_len, char* dst_str, size_t dst_len);
#endif

};

bool UserPwdValidByBearAuthorization(const httplib::Request& req, httplib::Response& res);
