# MediaGuard   
# 开发总结 2023-2-24  
完成HLS直播功能的开发。保留1分钟的文件，滚动的index list 8个文件，切片3秒。测试完成 2023-2-24。  
图片功能保存 默认全局设置是15分钟。并发条件变量：g_cvCond。（ManagerController.h）  
录像保存 全局设置默认容量，单位字节（Bytes）GlobalSetting.storageLimitedbytes 默认是根据已知道硬件作调整。  
GlobalSetting.nHDType 设置硬件类型，目标是显卡解码。StreamInfo.nHDType的设置是从这里获得。   

***
## 核心文件三个     

1. RtspStreamHandle.cpp 解码单元 對當前鏡頭單元解碼 MP4 FLV HLS JPEG RTMP等等  
对解码单元进行管理   

1. StreamManager.cpp  創建鏡頭【解码单元 】的日期文件夾等等 和當前鏡頭單元相關的 
整体控制    
 例如 StreamManager的構造函數
 ```C++
	switch (nStreamType)  
	{  
	case kStreamTypeUsb:  
		m_pHandle = std::make_shared<UsbStreamHandle>();  
		break;  
	case kStreamTypeAudio:  
		m_pHandle = std::make_shared<AudioStreamHandle>();  
		break;  
	case kStreamTypeRtsp:  
		m_pHandle = std::make_shared<RtspStreamHandle>();  
		break;  
	default:  
		m_pHandle = std::make_shared<RtspStreamHandle>();  
		break;  
	}
 ```
# struct StreamInfo （\MediaGuard\StreamDefine.h）
	1. hard code 實例要修改對應參數
	2. 如果要從雲端獲取則要從幾個API綜合獲取對應參數。
	3. API: CameraMpeg::camera_list；CameraMpeg::device_by_serial_no;CameraMpeg::setting_n_schedule_by_camera_id
	4. 還有一個是邏輯轉換的api camera_list_trans_to_strean_info， 主要是把雲端邏輯轉換為具體情況的：
	5. 例如：bRtmp = ttrue是開啟RTMP的，但新規則改為 StreamDecodeType (StreamInfo.nStreamDecodeType)的類型，HLS/RTMP/進行切換。
	6. 目前網上雲端沒有bRtmp對應的參數，對應的 CamSettingNSchedule。RtmpOutput和雲端對應的沒有UI改動的，只有默認FALSE.所有對RTMP比較混亂的，所以默認就是FALSE

---
通過共享指針實現獲取控制單元。

1. ManagerController.cpp 例如處理控制 保留全局文件夾Picture  

1. CameraMpeg.cpp 主要是API 業務邏輯的操作 記錄媒體文件開始結速時間等等的操作

***
# 开发 与 场景功能描述  
1. 实现NVR录像 云存储  
1. 各种识别业务与警报：  
1. 人脸识别/警报  
1. 二维码识别/警报  
1. 车牌识别/警报  
1. 老鼠识别/警报  
1. 行为识别/警报 
***
# 功能：  
 I:图片动态保存最近15分钟的图片，  
 II:动态把Camera解码单元（RtspStreamHandle.cpp）的单元图片列表转移保存到全局列表（内存）以供识别业务处理。  
 III:实现MP4/FLV 动态保存 
***
 #全局设置 
 GlobalSetting  来自/conf/Config.json 的全局配置。   
 1. picRemainMinutes;设置保存的图片在硬盘缓存时长；  
 1. videoRemainMinutes = 15; //无论超出容量限制都要最少保留15分钟的video，预设必须考虑最少可以存储15分钟的影片  
 1. HWDeviceType nHDType = kHWDeviceTypeNone; //硬件类型 默认是没有 
***
# 操作方法备注
***
 1. 增加Ctrl+c Ctrl+Break Enter 行为屏蔽（signal 监视）     
 1. 只能输入exit退出


***
![第一阶段测试界面](/TEST/wanchengdiyijieduanDEbug.jpg "Magic Gardens")

***
>MarkDown ref:https://markdown.com.cn/basic-syntax/horizontal-rules.html  

其他案例参考：  
https://www.cloudwalk.com/product/index2/id/16

***
# C++ 标准库智能指针  
使用这些智能指针作为将指针封装为纯旧 C++ 对象 (POCO) 的首选项。  

unique_ptr  
只允许基础指针的一个所有者。 除非你确信需要 shared_ptr，否则请将该指针用作 POCO 的默认选项。 可以移到新所有者，但不会复制或共享。 替换已弃用的 auto_ptr。 与 boost::scoped_ptr 比较。 unique_ptr 小巧高效；大小等同于一个指针且支持 rvalue 引用，从而可实现快速插入和对 C++ 标准库集合的检索。 头文件：<memory>。 有关详细信息，请参阅如何：创建和使用 unique_ptr 实例和 unique_ptr 类。

shared_ptr  
采用引用计数的智能指针。 如果你想要将一个原始指针分配给多个所有者（例如，从容器返回了指针副本又想保留原始指针时），请使用该指针。 直至所有 shared_ptr 所有者超出了范围或放弃所有权，才会删除原始指针。 大小为两个指针；一个用于对象，另一个用于包含引用计数的共享控制块。 头文件：<memory>。 有关详细信息，请参阅如何：创建和使用 shared_ptr 实例和 shared_ptr 类。

weak_ptr  
结合 shared_ptr 使用的特例智能指针。 weak_ptr 提供对一个或多个 shared_ptr 实例拥有的对象的访问，但不参与引用计数。 如果你想要观察某个对象但不需要其保持活动状态，请使用该实例。 在某些情况下，需要断开 shared_ptr 实例间的循环引用。 头文件：<memory>。 有关详细信息，请参阅如何：创建和使用 weak_ptr 实例和 weak_ptr 类。

MediaGuard 錄像系統的HTTP SERVER 請求與響應的測試，具體參考文檔：

[HttpServer_README.md]: MediaGuard/httpserver/HttpServer_README.md

 

<img src="TEST\test20230808\test20230808214611.jpg" style="zoom:67%;" />

<img src="TEST\test20230808\test20230808214612.jpg" alt="test20230808214612" style="zoom:67%;" />

<img src="TEST\test20230808\test20230808214619.jpg" alt="test20230808214619" style="zoom:67%;" />
