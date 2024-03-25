# MediaGuard PROJECT

# CMakeProject

備份在 https://www.123pan.com/

##### cmake ../ -G "Visual Studio 17 2022"

##### cmake ../ -G "Unix Makefiles"

#### 組件

MEDIA GUARD 項目主要 組件 : FFMPEG ;4.0  OPENCV 4.0

安裝 函數庫 用於 ctrl+break 等事件

ncurses 函式庫 $ sudo apt-get install libncurses5-dev

ncurses 函式庫是一個用於在終端窗口中創建文字用戶界面（TUI）的函式庫。它提供了一組函式和工具，用於管理終端的顯示，包括在屏幕上繪製文字、處理用戶輸入以及控制光標位置等。這使得開發人員能夠創建具有較高互動性的終端應用程序，例如文本編輯器、圖形化菜單和遊戲等。ncurses 函式庫通常用於 Unix-like 系統上，如 Linux 和 macOS。

 

#### 注意: MediaGuard_Cmake.RAR 不包含 3rdlib 子文件夾,由於太冗餘和龐大的文件數量,需要從 3rdlib.rar下載下來後 複製到根目錄下  3rdlib  : https://www.123pan.com/s/kDKPjv-Ed3l3.html 提取码:8uT1

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

#### 運行執行文件 executable (application/x-executable) 必須先指定共享文件(如 libswresample.so.4) FOR openssl-1.1.1q 

### **共享文件:**
`sudo ldconfig`
`export LD_LIBRARY_PATH=/home/tonylaw/Desktop/MediaGuard_Cmake:$LD_LIBRARY_PATH`
`./MediaGuard`

# **LINUX 運行的問題**

#### 设置共享库: 如 libswresample.so.4  具體筆記在 GOOGLE KEEP : LINUX\设置共享库: 如 libswresample.so.4



 工作log 

MediaGuard/src/interface/CameraMpeg.cpp

bool CameraMpeg::camera_list(Service::StreamInfoApiList& streamInfoApiList)  下的函數改造:

​	bool get_cam_succ = setting_n_schedule_by_camera_id(stream_camera_id, token1, cam_set);



CameraMpeg::detect_and_handle 未改為脫機和聯機 互相撤換模式

#### 函數 bool CameraMpeg::get_task_list_by_camera_id 需要加上model對象 FOR Yolov8:

 `"model_setting": [`

​          `{`

​              `"model_id": 1,`

​              `"model_name": "yolovx.pt",`

​              `"alert_trigger_conditions": false, //不是垃圾袋的情況`

​              `"precision_rate": 90,`

​              `"alert_voice": "請使用指定袋",`

​              `"alert_voice_file": "aaa.mp3"`

​          `}] //單個數組`





### device.json 配置節點 [carPlateRecogBusiness]

車牌識別：主要POST 到CarBusiness停車計時系統使用的。並且有具體的雲端API POST接口和限定鏡頭ID [permited_cameraId] list 等等

### 基於TASK 識別任務，如果Type_Mode 重複，則獲取的只會是updateTime最新的記錄。當前設計的任務識別功能有6大功能：

       1. 任務類型: UNDEFINED 未定義的任務
       UNDEFINED = 0,
    
       2. 任務類型:CAMERA_GUARD 桌面人臉識別系統  
        CAMERA_GUARD = 1, //不在任務中設置此功能
    
       3.  任務類型: CAMERA_DVR 桌面錄像系統
        CAMERA_DVR = 2,
    
       4. 任務類型: HIK_DATA_RETRIVE 桌面海康設備數據獲取系統
        DESKTOP_HIK_DATA_RETRIVE = 3,//不在任務中設置此功能
    
       5. 任務類型: HIK_DATA_ANDROID_RETRIVE  手機版的海康設備數據獲取系統
        ANDROID_HIK_DATA_RETRIVE = 4,//不在任務中設置此功能
    
       6. 任務類型: ANDROID_CIC_DATA_RETRIVE  手機版的 CIC的NFC拍卡數據獲取系統
        ANDROID_CIC_DATA_RETRIVE = 5, //不在任務中設置此功能
    
       7. 任務類型: 车牌识别 POST到Python处理后返回
        CAR_PLATE_RECOGNITION = 6,
    
       8. 任務類型: 人脸识别
        FACE_RECOGNITION = 7,
    
       9. 任務類型: 工程着装识别
        WORK_CLOTHES_RECOGNITION = 8,
    
      10. 任務類型: 佩戴头盔识别
        WEARING_HELMET_RECOGNITION = 9,
    
      11. 任務類型: 有人闖入
        SOMEONE_BROKE_IN = 10

2023-2-23  
HLS 保留20秒的切片文件（ts）记录 注意设置hls av_dist的时候不能大于这个时间，
2023-2-23  
HLS解码功能,完成，需要完善PTS和DTS精确度。需要增加删除清理过期的ts文件  
提示： pkt->duration = 0, maybe the hls segment duration will not precise  

2023-2-23    
HLS解码功能，无法进入解码流，exit退出后才能进入hls解码流输出，但由于系统任意键主线程几秒后退出，导致主线程停止，应该是之前的RtspStreamHandle.cpp内的其他输出解码流线程阻塞导致。

2023-2-13  
//解决pts dts 一开始出现负数的问题  

//关于ffmpeg 解码学习的代码干货：  
https://www.cnblogs.com/gongluck/archive/2019/05/07/10827950.html  

//關於MediaGuard.exe是否佔用端口可使用命令：netstat -anb
####   

# 跨平台开发 
参考 https://www.cnblogs.com/end-emptiness/p/15083829.html

----------------------------------------------------------
# unicode开发
为了避免不同平台编码不同导致的乱码问题，通常会转成unicode编码来做

首先需要切换解决方案的字符集：
配置属性 -> 常规 -> 字符集 -> 使用 Unicode 字符集

cmake指令是：add_definitions(-DUNICODE -D_UNICODE)

类型：wstring（字符串）, wchar_t（字符）

# 常量字符初始化
wstring a = L"啊啊啊啊啊";
wchar_t b[] = {L'哈', L'呵'};  

# vs自带功能通过ssh+gdb自动远程调试

在Visual Studio中，可以通过SSH直接连接到远程计算机并使用GDB进行自动远程调试。以下是如何设置和启动远程调试会话的步骤：

1. 打开Visual Studio。
2. 创建一个新的项目或打开现有的项目。
3. 右键点击解决方案资源管理器中的项目，选择“属性”。
4. 导航到“配置属性” -> “调试”。
5. 在“远程调试器”部分，设置“远程命令”为GDB的路径，例如：`/usr/bin/gdb`。
6. 设置“远程服务器名称”为远程设备的IP地址或主机名。
7. 在“远程路径”中，指定远程机器上可执行文件的路径。
8. 在“调试器类型”中，选择“远程（GDB）”。
9. 确保在“调试器选项”中，你可以指定任何GDB特定的启动参数，如指定特定的GDB客户端端口。
10. 配置SSH连接，确保你的远程机器上安装了SSH服务，并且你有权限通过SSH连接。
11. 在“配置属性” -> “链接器” -> “调试”中，设置“生成调试信息”为“是”。
12. 在“系统属性”中，确保目标机器的架构（x86或x64）与你的本地机器相匹配。
13. 点击“确定”保存设置。
14. 点击“本地Windows调试器”旁边的下拉菜单，选择“远程Windows调试器”或“远程Linux调试器”，取决于你的远程系统类型。
15. 点击“开始调试”或按F5开始远程调试会话。

注意：确保远程机器上已经安装了GDB，并且你有足够的权限来执行GDB和调试你的程序。如果你的程序依赖于特定的库或环境，你需要确保这些在远程机器上也是可用的。

### 更新一個boost 1_63_0庫 到

tar -zxvf boost_1_63_0.tar.gz
cd boost_1_63_0
#编译boost库，全库编译
./bootstrap.sh --with-toolset=gcc link=static threading=multi runtime-link=static --with-libraries=all

####  boost 1_83_0庫 指令

tar -zxvf boost_1_83_0.tar.gz
cd boost_1_83_0

./bootstrap.sh --with-toolset=gcc link=static threading=multi runtime-link=static --with-libraries=all

