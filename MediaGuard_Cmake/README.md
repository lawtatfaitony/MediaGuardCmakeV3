# CMakeProject

備份在 https://www.123pan.com/

##### cmake ../ -G "Visual Studio 17 2022"

##### cmake ../ -G "Unix Makefiles"

#### 組件

MEDIA GUARD 項目主要 組件 : FFMPEG ;4.0  OPENCV 4.0

安裝 函數庫 用於 ctrl+break 等事件

ncurses 函式庫 $ sudo apt-get install libncurses5-dev

ncurses 函式庫是一個用於在終端窗口中創建文字用戶界面（TUI）的函式庫。它提供了一組函式和工具，用於管理終端的顯示，包括在屏幕上繪製文字、處理用戶輸入以及控制光標位置等。這使得開發人員能夠創建具有較高互動性的終端應用程序，例如文本編輯器、圖形化菜單和遊戲等。ncurses 函式庫通常用於 Unix-like 系統上，如 Linux 和 macOS。

 

#### 注意: MediaGuard_Cmake.RAR 不包含 3rdlib 子文件夾,由於太冗餘和龐大的文件數量,需要從 3rdlib.rar下載下來後 複製到根目錄下

### 頭文件

重要文件: Common.h  連接相關多個頭文件



參考如何Linux ffmpeg

https://blog.csdn.net/mao_hui_fei/article/details/132192108

# openssl-1.1.1q出現問題導致 

##### 先屏蔽了 std::string md5_lower_string = md5_stringstream.str();  // Cmd5::get_md5(md5_stringstream.str());  //引用openssl1.1.1q

`/usr/bin/ld: /home/tonylaw/Desktop/MediaGuard_Cmake/3rdlib/linux/x64/openssl-1.1.1q/libcrypto.so.1.1:`

 error adding symbols: DSO missing from command line

**https://blog.csdn.net/q7w8e9r4/article/details/134631522**
<u>`error adding symbols: DSO missing from command line`</u>

上述參考: LINUX\设置共享库 或參考 add_executable(your_target_name your_source_file.cpp -lcrypto)



# /*LINUX 運行的問題*/

#### 设置共享库: 如 libswresample.so.4  具體筆記在 GOOGLE KEEP : LINUX\设置共享库: 如 libswresample.so.4



 工作log 

MediaGuard/src/interface/CameraMpeg.cpp

bool CameraMpeg::camera_list(Service::StreamInfoApiList& streamInfoApiList)  下的函數改造:

​	bool get_cam_succ = setting_n_schedule_by_camera_id(stream_camera_id, token1, cam_set);



CameraMpeg::detect_and_handle 未改為脫機和聯機 互相撤換模式

#### 函數 bool CameraMpeg::get_task_list_by_camera_id 需要加上model對象:

 "model_setting": [

​          {

​              "model_id": 1,

​              "model_name": "yolovx.pt",

​              "alert_trigger_conditions": false, //不是垃圾袋的情況

​              "precision_rate": 90,

​              "alert_voice": "請使用指定袋",

​              "alert_voice_file": "aaa.mp3"

​          }] //單個數組

