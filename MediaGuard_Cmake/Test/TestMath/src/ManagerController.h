#pragma once
#include <iostream>  
#include <filesystem> 
 
#include "Time.h"
#include <condition_variable>
#include <thread>

#define VERIFY_RETURN(expr) \
    do {    \
        auto ret = expr;    \
        if(!ret) {  \
            return false;   \
        }   \
    } while (false);

#define VERIFY_RETURN_VOID(expr) \
    do {    \
        auto ret = expr;    \
        if(!ret) {  \
            return;   \
        }   \
    } while (false);


#define ENUM_INITIALIZE() \
   {    \
        CameraConnectingStatus CameraConnectingStatus[0];  \
        AVCodecID AVCodecID[AV_CODEC_ID_AAC];   \
   }    \

#define GUARD_LOCK(lock)    \
    std::lock_guard<std::mutex> locker(lock);

#define SHARED_LOCK(lock)    \
    std::unique_lock<std::mutex> locker(lock);



class ManagerController
{
public:
    ManagerController();
    ~ManagerController();
public:  
    void run(); 

public:
    std::string kVideoDir;
    std::string kPictureDir;
    std::string kAudioDir;
    std::string kHlsDir;

    std::atomic<bool> g_bInited;
    std::atomic<bool> g_bExit;
    std::atomic<bool> main_exit; //通过信号判断是否退出
    std::mutex g_mtLock;
    std::condition_variable g_cvCond;
    std::thread g_thRun;
    std::thread g_thPict;
    std::thread g_thVideoStore; 
};

