//
//  HttpServer.cc  from https://gitee.com/AIQICcorg/cpp-httplib support
//
//  Copyright (c) 2019 Yuji Hirose. All rights reserved.
//  MIT License
#pragma once

#include <chrono>  //from httpcgi example 2024-3-10
#include <cstdio>  //from httpcgi example 2024-3-10

#include "httplib/httplib.h"
#include "httpserver.h"
#include "../httpserver/comm.h"
#include "../Common.h"
#include "../hmac/hmac_sha1.h"
#include "Common/Cmd5.h"
#include "Config/DeviceConfig.h"
 
 
//httpserver::httpserver() {
//	 
//}
//httpserver::~httpserver()
//{
//}

void allowCorsAccess(httplib::Response& res) {
	// 允许跨域请求
	res.set_header("Access-Control-Allow-Origin", "*");
	res.set_header("Access-Control-Allow-Credentials", "true");
	res.set_header("Access-Control-Allow-Private-Network", "true");
	res.set_header("Access-Control-Allow-Methods", "POST, GET, OPTIONS, PUT, DELETE, PATCH");
	//res.set_header("Access-Control-Allow-Headers", "*"); //添加這個特定的試試，之前是注釋掉的 2023-8-6

	//std::stringstream allow_req_uri;
	//{allow_req_uri << DEVICE_CONFIG.cfgHttpServer.scheme << "://" << DEVICE_CONFIG.cfgHttpServer.host << ":" << DEVICE_CONFIG.cfgHttpServer.port; }
	//res.set_header("Access-Control-Allow-Origin", allow_req_uri.str());

	res.set_header(
		"Access-Control-Allow-Headers",
		"X-Custom-Header,DNT,User-Agent,X-Requested-With,If-Modified-Since,Cache-"
		"Control,Content-Type,Range,Accept,Accept-Encoding,Accept-Language,"
		"Access-Control-Request-Headers,Access-Control-Request-Method,Connection,"
		"Host,Origin,Sec-Fetch-Mode,Referer,Authorization");
}

std::string dump_headers(const httplib::Headers& headers) {
	std::string s;
	char buf[BUFSIZ];

	for (auto it = headers.begin(); it != headers.end(); ++it) {
		const auto& x = *it;
		snprintf(buf, sizeof(buf), "%s: %s\n", x.first.c_str(), x.second.c_str());
		s += buf;
	}

	return s;
}

std::string log(const httplib::Request& req, const httplib::Response& res) {
	std::string s;
	char buf[BUFSIZ];

	s += "\n-------------------------------------------------\n";

	snprintf(buf, sizeof(buf), "%s %s %s", req.method.c_str(),
		req.version.c_str(), req.path.c_str());
	s += buf;

	std::string query;
	for (auto it = req.params.begin(); it != req.params.end(); ++it) {
		const auto& x = *it;
		snprintf(buf, sizeof(buf), "%c%s=%s",
			(it == req.params.begin()) ? '?' : '&', x.first.c_str(),
			x.second.c_str());
		query += buf;
	}
	snprintf(buf, sizeof(buf), "%s\n", query.c_str());
	s += buf;

	s += dump_headers(req.headers);

	s += "\n-------------------------------------------------\n";

	snprintf(buf, sizeof(buf), "%d %s\n", res.status, res.version.c_str());
	s += buf;
	s += dump_headers(res.headers);
	s += "\n";

	if (!res.body.empty()) { s += res.body; }

	s += "\n";

	return s;
}

// 校验token參數
bool UserPasswordValidByToken(const httplib::Request& req, httplib::Response& res) {

	// 认证用户秘钥
	std::string token = req.get_param_value("token");
	std::string local_authorization = DEVICE_CONFIG.cfgDevice.local_authorization;
	//printf("\nUserPasswordValidByToken  token = %s\n\n config local_authorization =%s\n\n", token.c_str(), local_authorization.c_str());

	if (token.empty()) {
		res.status = 401;
		res.set_content("No Authentic 401 (token empty)", "text/plain");
		return false;
	}

#ifdef _WIN32
	if (!token._Equal(local_authorization)) {
		res.status = 401;
		return false;
	}
	else {
		return true;
	}
#elif __linux__
	// 请自行补充
	return false;
#endif
}

/// <summary>
/// 校验用户名密码以Bearer Token 形式
/// 用户名密碼保存在 ./conf/device.json下的節點device_config.local_login:device_config.local_password
/// </summary>
/// <param name="req"></param>
/// <param name="res"></param>
/// <returns></returns>
bool UserPwdValidByBearAuthorization(const httplib::Request& req, httplib::Response& res)
{
	// 认证用户秘钥
	std::string authentication = req.get_header_value("Authorization");
	if (authentication.empty())
	{
		res.status = 401;
#ifdef _WIN32
		res.set_content(transcoding::T2UTF8("No Header Authorization To Access!").c_str(), "text/plain");
#elif __linux__
		// 请自行补充
#endif
		return false;
	}

	// 认证用户秘钥
	std::string local_login = DEVICE_CONFIG.cfgDevice.local_login;
	std::string local_password = DEVICE_CONFIG.cfgDevice.local_password;

	std::stringstream md5_stringstream;
	{ md5_stringstream << local_login << ":" << local_password; }
  
	std::string md5_lower_string = md5_stringstream.str();  // ★★★★★★ Cmd5::get_md5(md5_stringstream.str());  //需要用openssl1.1.1q,後續改回來, 不使用md5 則和雲端密碼驗證對不上的.
	//========================================================================================================================================
	//openssl-1.1.1q出現問題導致
	//先屏蔽了 std::string md5_lower_string =  Cmd5::get_md5(md5_stringstream.str()); // LINUX 編譯出錯
	///usr/bin/ld: /home/tonylaw/Desktop/MediaGuard_Cmake/3rdlib/linux/x64/openssl-1.1.1q/libcrypto.so.1.1:
	//error adding symbols : DSO missing from command line
	//https ://blog.csdn.net/q7w8e9r4/article/details/134631522 error adding symbols: DSO missing from command line
	//上述參考: LINUX\设置共享库 或參考 add_executable(your_target_name your_source_file.cpp - lcrypto)



	std::string local_authorization = DEVICE_CONFIG.cfgDevice.local_authorization;

#ifdef _WIN32
	if (!authentication._Equal(local_authorization))
	{
		res.status = 401;
		res.set_content(transcoding::T2UTF8("not authorized to access this interface(authentication no equal)!").c_str(), "text/plain");
		return false;
	}
	return true;

#elif __linux__
	// 请自行补充
	return false;
#endif

}
//初始化和定義
int httpserver::http_server_run(void) {

//http server
httplib::Server svr;



	if (!svr.is_valid()) {
		printf("server has an error...\n");
		return -1;
	}
	// User defined file extension and MIME type mappings
	svr.set_file_extension_and_mimetype_mapping("txt", "text/plain");
	svr.set_file_extension_and_mimetype_mapping("md", "text/plain");
	svr.set_file_extension_and_mimetype_mapping("html", "text/html");
	svr.set_file_extension_and_mimetype_mapping("htm", "text/html");
	svr.set_file_extension_and_mimetype_mapping("bmp", "image/bmp");
	svr.set_file_extension_and_mimetype_mapping("jpg", "image/jpeg");
	svr.set_file_extension_and_mimetype_mapping("jpeg", "image/jpeg");

	svr.set_file_extension_and_mimetype_mapping("mp3", "video/mp3");
	svr.set_file_extension_and_mimetype_mapping("mp4", "video/mp4");
	svr.set_file_extension_and_mimetype_mapping("flv", "video/x-flv");
	svr.set_file_extension_and_mimetype_mapping("mpeg", "video/mpeg");
	svr.set_file_extension_and_mimetype_mapping("mp3", "audio/mp3");
	svr.set_file_extension_and_mimetype_mapping("avi", "video/x-msvideo");
	svr.set_file_extension_and_mimetype_mapping("m3u8", "application/vnd.apple.mpegurl");
	svr.set_file_extension_and_mimetype_mapping("ts", "video/mp2t");

	svr.Get("/", [=](const httplib::Request& /*req*/, httplib::Response& res) {
		res.set_redirect("/hi");
		});

#ifdef _WIN32
	svr.set_pre_routing_handler(
		[](const auto& req, auto& res) -> httplib::Server::HandlerResponse {
			if (req.path == "/hello") { //全局處理路由

				//<meta charset="UTF-8">
				std::string templateHtml = R"(
                        <!DOCTYPE html>
                        <html lang="en">
                        <head> 
							<title>Hello the world</title>
							<meta http-equiv="Content-Type" content="text/html;charset=UTF-8"/>
                        </head>
                        <body>
                        <h1> $hello$ </h1>
                        <script>
                        const ev1 = new EventSource("event1");
                        ev1.onmessage = function(e) {
                          console.log('ev1', e.data);
                        }
                        const ev2 = new EventSource("event2");
                        ev2.onmessage = function(e) {
                          console.log('ev2', e.data);
                        }
                        </script>
                        </body>
                        </html>
                        )";
				cout << "the current page is basic on utf8" << endl;
				const char* src_char = "Hello the world,您好，世界";
				cout << "origin string: " << src_char << endl;

#ifdef _WIN32
				// windows default is gbk
				/*string dst_str = httpserver::GbkToUtf8(src_char);
				cout << "gbk to utf8: " << dst_str << endl;*/

				string dst_str = src_char; //本來頁面是Utf8,並且要求主控台也是Utf8,所以無須轉碼。


				string str_utf8 = httpserver::Utf8ToGbk(dst_str.c_str());
				cout << "utf8 to gbk: " << str_utf8 << endl;
#else
				// unix default is utf8
				char dst_gbk[1024] = { 0 };
				Utf8ToGbkx(src_str, strlen(src_str), dst_gbk, sizeof(dst_gbk));
				cout << "utf8 to gbk: " << dst_gbk << endl;

				char dst_utf8[1024] = { 0 };
				GbkToUtf8x(dst_gbk, strlen(dst_gbk), dst_utf8, sizeof(dst_utf8));
				cout << "gbk to utf8: " << dst_utf8 << endl;
#endif  

				std::string stringToken = "$hello$";

				std::string newstr = templateHtml.replace(
					templateHtml.find(stringToken),
					stringToken.length(),
					src_char); //str_utf8

				cout << "templateHtml: " << templateHtml.c_str() << endl;

				res.set_content(templateHtml.c_str(), "text/html");

				return httplib::Server::HandlerResponse::Handled;
			}
			return httplib::Server::HandlerResponse::Unhandled;
		});
#elif __linux__
	// 请自行补充
#endif

	svr.Get("/hi", [](const httplib::Request& /*req*/, httplib::Response& res) {
		res.set_content("Hello World!\n", "text/plain");
		});

	svr.Get("/slow", [](const httplib::Request& req, httplib::Response& res) {
		std::this_thread::sleep_for(std::chrono::seconds(2));

		if (req.has_param("key")) { auto val = req.get_param_value("key"); }
		res.set_content("Slow...\n", "text/plain");
		});

	// 探测请求
	svr.Options(R"(/video/(\d+)-(\d+)-(\d+))",
		[](const httplib::Request& req, httplib::Response& res) {
			// 允许跨域请求
			allowCorsAccess(res);
			printf("Video Options");
		});

	svr.Get(R"(/video/(\d+)-(\d+)-(\d+))", [&](const httplib::Request& req, httplib::Response& res) {
		if (req.has_param("token")) {
			auto token = req.get_param_value("token");
			std::stringstream authentic_content;
			{ authentic_content << "Athentic token = " << token; }
			printf(authentic_content.str().c_str());

			if (UserPasswordValidByToken(req, res) == false) {
				printf("video req.path = %s\n\n", req.path.c_str());
				return;
			}
		}
		else {

			std::string msg = "<p>User Authentic Status: <span "
				"style='color:red;'>%d</span> </p> <p>不支持認證</p>";
			const char* fmt = msg.c_str();
			char buf[BUFSIZ];
			res.status = 401;
			snprintf(buf, sizeof(buf), fmt, res.status);
			res.set_content("No Atuthentic : User Authentic Status:401", "text/html");
		}
		});

	// 探测请求
	svr.Options("/video",
		[](const httplib::Request& req, httplib::Response& res) {
			// 允许跨域请求
			allowCorsAccess(res);
			printf("Video Options");
		});
	// authentic with token
	svr.Get("/video", [](const httplib::Request& req, httplib::Response& res) {
		if (req.has_param("token")) {
			auto token = req.get_param_value("token");
			std::stringstream authentic_content;
			{ authentic_content << "Athentic token = " << token; }
			printf(authentic_content.str().c_str());

			if (UserPasswordValidByToken(req, res) == false) {
				printf("video req.path = %s\n\n", req.path.c_str());
				return;
			}
		}
		else {

			std::string msg = "<p>User Authentic Status: <span "
				"style='color:red;'>%d</span> </p> <p>不支持認證</p>";
			const char* fmt = msg.c_str();
			char buf[BUFSIZ];
			res.status = 401;
			snprintf(buf, sizeof(buf), fmt, res.status);
			res.set_content("No Atuthentic : User Authentic Status:401", "text/html");
		}
		});
	// 探测请求
	svr.Options("/video", [](const httplib::Request& req, httplib::Response& res) {
		// 允许跨域请求
		allowCorsAccess(res);
		});

	// 探测请求
	svr.Get("/play", [](const httplib::Request& req, httplib::Response& res) {
		allowCorsAccess(res);
		res.status = 200;
		res.set_content("{'success':true,errorCode:-1}", "text/html");
		});
	svr.Options("/play", [](const httplib::Request& req, httplib::Response& res) {
		// 允许跨域请求
		allowCorsAccess(res);
		});

	// 播放视频
	//svr.Get("/play", [](const httplib::Request& req, httplib::Response& res) {
	//	// 允许跨域请求
	//	allowCorsAccess(res);
	//	// 用户名密码校验
	//	if (!UserPasswordValidByHeader(req, res))
	//	{
	//		return;
	//	}

	//	//DOC: http://192.168.8.19:180/play?deviceId=11&channel=1&stream=1
	//	// 获取播放参数
	//	std::string deviceId = req.get_param_value("deviceId");
	//	std::string channel = req.get_param_value("channel");
	//	std::string stream = req.get_param_value("stream");

	//	std::stringstream mpeg_path;
	//	{mpeg_path <<  File::GetWorkPath() << "\\hls\\" << deviceId << "\\index.m3u8"; } 
	//	});

	svr.Get("/dump", [](const httplib::Request& req, httplib::Response& res) {
		res.set_content(dump_headers(req.headers), "text/plain");
		});

	svr.Get("/stop",
		[&](const httplib::Request& /*req*/, httplib::Response& /*res*/) { svr.stop(); });

	// 更新录像策略
	httplib::Server::Handler updateStrategy = [&](const httplib::Request& req, httplib::Response& res) {
		/*
		{
			"id": "13",
			"saveMpeg4": false,
			"sundayStart": "00:00:00",
			"sundayEnd": "23:59:59",
			"mondayStart": "00:00:00",
			"mondayEnd": "23:59:59",
			"tuesdayStart": "00:00:00",
			"tuesdayEnd": "23:59:59",
			"wednesdayStart": "00:00:00",
			"wednesdayEnd": "23:59:59",
			"thursdayStart": "00:00:00",
			"thursdayEnd": "23:59:59",
			"fridayStart": "00:00:00",
			"fridayEnd": "23:59:59",
			"saturdayStart": "00:00:00",
			"saturdayEnd": "23:59:59"
		}
		*/
		// 允许跨域请求
		allowCorsAccess(res);
#ifdef _WIN32
		res.set_content(transcoding::T2UTF8("add strategy success!").c_str(), "text/plain");
#elif __linux__
		// 请自行补充
#endif
	};

	svr.Post("/updateStrategy", updateStrategy);

	svr.set_logger([](const httplib::Request& req, const httplib::Response& res) {
		if (res.status == 404 || res.status == 401 || res.status == 302 || res.status == 500)
		{
			printf("\n [http_server_logger] [log] [StatusCode=404||401||302||500] res.status = %d \n%s", res.status, log(req, res).c_str());
		}
		else {
			//printf("\n [http_server_logger] request_status_code = %d\n", res.status);
		}
		});
	 
	auto ret1 = svr.set_mount_point("/video", "./video");

	if (!ret1) {
		printf("./video does not exist! %s\n\n", "./video");
	}
	else {
		printf("mount ./video success! %s\n\n", "./video");
	}

	 
	auto ret2 = svr.set_mount_point("/hls", "./hls");
	if (!ret2) {
		printf("./hls does not exist! %s\n\n", "./hls");
	}
	else {
		printf("mount ./hls success! %s\n\n", "./hls");
	}

	// 探测请求
	svr.Options("/hls", [](const httplib::Request& req, httplib::Response& res) {
		// 允许跨域请求
		allowCorsAccess(res);
		printf("hls Options");
		});

	std::stringstream picture_path;
	{ picture_path << File::GetWorkPath() << kSeprator << "picture"; }
	auto ret3 = svr.set_mount_point("/picture", "./picture");
	if (!ret3) {
		printf("./picture does not exist! %s\n\n", "./picture");
	}
	else {
		printf("mount ./picture success! %s\n\n", "./picture");
	}
	 
	auto ret4 = svr.set_mount_point("/web", "./web");
	if (!ret4) {
		printf("./web does not exist! %s\n\n", "./web");
	}
	else {
		printf("mount ./web success!%s\n\n", "./web");
	}
	/// mount end

	// 文件请求响应处理器(请求成功响应之前)
	svr.set_file_request_handler(
		[](const httplib::Request& req, httplib::Response& res) {
			// 允许跨域请求
			allowCorsAccess(res);
			// 获取请求路径：/111/1/0/index.m3u8
			std::string path = req.path;
			//printf("\nfile_request path = %s\n\n", path.c_str());

			//// html/htm文件免用户校验
			if (path.find(".html") != -1 || path.find(".htm") != -1)  //需要token
			{
				//開關token驗證
				if (!UserPasswordValidByToken(req, res)) {
					printf("request html reuired token ,html unauthorized!!!(set_file_request_handler %s)\n\n", path.c_str());
					return;
				}
			}

			//// ts文件免用户校验
			if (path.find(".ts") == -1)
			{
				if (!UserPasswordValidByToken(req, res)) {
					printf(".ts unauthorized!!! set_file_request_handler %s\n\n", path.c_str());
					printf("Content-Type video/mp2t %s\n\n", path.c_str());
					return;
				}
			}

			//// mp4索引文件下载
			if (path.find(".mp4") != -1) {
				if (UserPasswordValidByToken(req, res) == false) {
					printf("mp4 unauthorized!!! set_file_request_handler %s\n\n", path.c_str());
					return;
				}
			}
			////// m3u8索引文件下载
			//if (path.find("index.m3u8") != -1) {  //path.npos
			//	printf("Content-Type application/vnd.apple.mpegurl %s\n\n", path.c_str());
			//	res.set_header("Content-Type", "application/vnd.apple.mpegurl");
			//}
		});

#ifdef _WIN32
	// 错误处理
	httplib::Server::Handler errorHandle = [](const auto& req, auto& res) {
		auto fmt = "<p>Error Status: <span style='color:red;'>media server error: %d</span></p>";
		char buf[BUFSIZ];
		snprintf(buf, sizeof(buf), fmt, res.status);
		res.set_content(buf, "text/html");
	};
	svr.set_error_handler(errorHandle);
#elif __linux__
	// 请自行补充
#endif

	//---------------------------------------------------------------------------------------------
	std::string local_ip = DEVICE_CONFIG.cfgDevice.device_ip;
	const int local_port = DEVICE_CONFIG.cfgDevice.device_port;;
	svr.listen(local_ip.c_str(), local_port);

	return 0;
}

#ifdef _WIN32
#include <windows.h>
#include <string>

string httpserver::GbkToUtf8(const char* src_str) {
	int len = MultiByteToWideChar(CP_ACP, 0, src_str, -1, NULL, 0);
	wchar_t* wstr = new wchar_t[len + 1];
	memset(wstr, 0, len + 1);
	MultiByteToWideChar(CP_ACP, 0, src_str, -1, wstr, len);
	len = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, 0, NULL, NULL);
	char* str = new char[len + 1];
	memset(str, 0, len + 1);
	WideCharToMultiByte(CP_UTF8, 0, wstr, -1, str, len, NULL, NULL);
	string strTemp = str;
	if (wstr) delete[] wstr;
	if (str) delete[] str;
	return strTemp;
}

string httpserver::Utf8ToGbk(const char* src_str) {
	int len = MultiByteToWideChar(CP_UTF8, 0, src_str, -1, NULL, 0);
	wchar_t* wszGBK = new wchar_t[len + 1];
	memset(wszGBK, 0, len * 2 + 2);
	MultiByteToWideChar(CP_UTF8, 0, src_str, -1, wszGBK, len);
	len = WideCharToMultiByte(CP_ACP, 0, wszGBK, -1, NULL, 0, NULL, NULL);
	char* szGBK = new char[len + 1];
	memset(szGBK, 0, len + 1);
	WideCharToMultiByte(CP_ACP, 0, wszGBK, -1, szGBK, len, NULL, NULL);
	string strTemp(szGBK);
	if (wszGBK) delete[] wszGBK;
	if (szGBK) delete[] szGBK;
	return strTemp;
}

#else
#include <iconv.h>
#include <string.h>
int httpserver::GbkToUtf8(char* str_str, size_t src_len, char* dst_str, size_t dst_len) {
	iconv_t cd;
	char** pin = &str_str;
	char** pout = &dst_str;

	cd = iconv_open("utf8", "gbk");
	if (cd == 0) return -1;
	memset(dst_str, 0, dst_len);
	if (iconv(cd, pin, &src_len, pout, &dst_len) == -1) return -1;
	iconv_close(cd);
	 
	dst_str = new char('\0');  // 創建一個包含空字符的 char* 對象
	pout = &dst_str;  // 將指向 char* 的指針賦值給 char**

	return 0;
}

int httpserver::Utf8ToGbk(char* src_str, size_t src_len, char* dst_str, size_t dst_len) {
	iconv_t cd;
	char** pin = &src_str;
	char** pout = &dst_str;

	cd = iconv_open("gbk", "utf8");
	if (cd == 0) return -1;
	memset(dst_str, 0, dst_len);
	if (iconv(cd, pin, &src_len, pout, &dst_len) == -1) return -1;
	iconv_close(cd);

	dst_str = new char('\0');  // 創建一個包含空字符的 char* 對象
	pout = &dst_str;  // 將指向 char* 的指針賦值給 char**

	return 0;
}

#endif
