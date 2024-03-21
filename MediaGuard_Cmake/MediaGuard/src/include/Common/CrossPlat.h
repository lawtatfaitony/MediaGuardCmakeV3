#pragma once
#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#endif // _WIN32




#ifdef _WIN32
#define CD_access(file,mode) _access(file, mode)
#define CD_local_time(time,tm) localtime_s(&tm, &time)
#define CD_mkdir(dir) _mkdir(dir)


#else
#define CD_access(file,mode) access(file, mode)
#define CD_local_time(time,tm) localtime_r(&time, &tm)
#define CD_mkdir(dir) mkdir(dir, 775)
#endif // _WIN32

