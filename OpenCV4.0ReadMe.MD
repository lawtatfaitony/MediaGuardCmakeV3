# OPENCV 4.0    

# 版本4.0   
	 https://opencv.org/opencv-4-0-0-alpha.html

OpenCV 4.0 alpha包括3.4分支的所有最新改进，优化和 Bug修复。尤其是：

ONNX解析器已添加到OpenCV DNN模块中。它支持各种分类网络，如AlexNet，Inception v2，Resnet，VGG等。还支持 tiny YOLO v2对象检测网络。
其他一些显著的DNN改进：
Mask RCNN 支持和示例
Faster R-CNN：使用Intel Inference Engine（英特尔OpenVINO的一部分）加速
基于OpenCL backend的几个稳定性改进。
快速QR码检测器(detector)（Core i5 desktop的~80FPS @ 640x480分辨率）。官方计划在OpenCV 4.0正式版中添加QR码解码器(decoder)，以便有一个完整的解决方案。
通过所谓的“wide universal intrinsics”不断扩展SSE4，AVX2和NEON优化内核集。
此外，OpenCV 4.0 alpha包含一些独有的功能，例如：

OpenCV现在是基于C ++ 11库，它需要符合C ++ 11标准的编译器。因此，默认情况下可以使用一些很好的功能，例如parallel_for和lambda函数，方便迭代cv :: Mat，初始化cv :: Mat，通过列出它的元素等。
标准的std :: string和std :: shared_ptr取代了手工制作的cv :: String和cv :: Ptr。我们的parallel_for现在可以使用std :: threads pool 作为后端。
OpenCV 1.x的旧版C API（使用CvMat，IplImage等）被部分排除在外；cleanup 应该主要由OpenCV 4.0 正式版完成。
添加了基本的FP（float point）16支持（添加了新的CV_16F类型）。 CPU和GPU加速的KinFu实时三维密集重建算法已包含在opencv_contrib中。
HPX parallel backend（感谢Jakub Golinowski）
新的chessboard detector（棋盘探测器）（感谢Alexander Duda）
总的来说，OpenCV 4.0 alpha版本包括85个补丁，包括来自3.4分支的28个大规模合并请求。

# OpenCV 4.0-beta 新特性  
https://opencv.org/opencv-4-0-0-beta.html

OpenCV 4.0 Beta 包括许多新功能和增强功能。除了这个新的库，还有新的开源工具OpenVINO™（Open Visual Inference and Neural Network Optimization），可以帮助帮助快速跟踪高性能计算机视觉开发和深度学习推理。

OpenCV 4.0 Beta包括29个新补丁，包括自OpenCV 4.0 alpha 以来3.4分支的大规模合并：

ONNX *（Open Neural Network Exchange）importer 已得到进一步改进，以支持更多扩展
OpenCV DNN示例object_detection.py已经改进，可以填写正确的模型参数，因此现在使用起来要容易得多
G-API（Graph API） - 超高效图像处理 pipeline 引擎已集成为opencv_gapi模块
快速QR码解码器（decoder），该算是是基于免费的QUirc（https://github.com/dlbeer/quirc）库，所以现在我们有一个完整的QR码检测和解码实现（运行~20-80FPS @ 640x480分辨率）
使用“wide universal intrinsics”为AVX2加速了超过60个内核的18个功能
针对iGPU加速了Kinect Fusion算法，在高分辨率 volume（512x512x512）上实现了并行CPU版本的3倍加速
OpenCV 4.0-rc 新特性
https://opencv.org/opencv-4-0-0-rc.html

OpenCV 4.0-rc 相比OpenCV 4.0 beta新增了约60个补丁。

将实验中的Vulkan backend添加到DNN模块
opencv_stitching模块接口被重构
实现了更加精确的相机镜头校正算法
核心模块的部分函数（即以XML / YML / JSON格式存储数据到硬盘）已在C ++中重写，使得在读取FileStorage时占用的内存较少。 C语言风格的API（CvFileStorage）已被删除
扩展了Graph API模块，使其获得初始异构支持，OpenCL支持，文档也更加完善，并支持独立构建它
删除了以下模块的C语言风格API：photo, video, imgcodecs, videoio
删除了videoio 与 highgui 模块中过时的后端支持，包括: QuickTime, QTKit, Unicap, Video for Windows, libv4l, DC1394_V1, Carbon
-shape, superres, videostab, viz 模块和 TVL1 光流算法被移动到opencv_contrib

DIS 光流算法从opencv_contrib被移动到main repository
包含OpenCV 3.4版本的一些最新改进，比如：libpng更新 (安全修复), 支持XCode 10, 支持Turing GPU, 许多优化和Bug 修复
OpenCV 4.0 正式版新特性
https://opencv.org/opencv-4-0-0.html

OpenCV现在基于C++ 11库，需要符合C++ 11 标准的编译器。所需的最低CMake版本已提升至3.5.1。
很多来自OpenCV 1.x的C API已被删除。
在core模块中的部分功能（如在XML，YAML或JSON中存储和加载结构化数据）已在C++中完全重新实现，并且也删除了C API。
添加了新的模块G-API，它可以作为非常有效的基于图形的图像处理 pipeline的引擎。
dnn模块使用OpenVINO™工具包R4中的Deep Learning Deployment Toolkit进行了更新。请参阅指南如何构建和使用支持DLDT的OpenCV。
dnn模块现在包括实验性Vulkan backend，并支持ONNX格式的网络。
Kinect Fusion算法已针对CPU和GPU（OpenCL）进行实现和优化
QR码检测器和解码器已添加到objdetect模块中
非常高效且高质量的DIS密集光流算法已从opencv_contrib转移到video模块。
更多细节可以在之前的宣布中找到：4.0-alpha，4.0-beta，4.0-rc和更新日志
# OpenCV 4.0 更新日志
https://github.com/opencv/opencv/wiki/ChangeLog#version400

OpenCV 4.0-alpha：2018年9月

OpenCV 4.0-beta：2018年10月

OpenCV 4.0-final：2018年11月

# 下面是介绍OpenCV 4.0新特性：

删除大量OpenCV 1.x 遗留的C语言风格的API接口，主要模块是objdetect, photo, video, videoio, imgcodecs, calib3d
在core模块中的部分函数（在XML，YAML或JSON中存储和加载结构化数据）已用C++ 重新实现，同时删除了C 风格的API。 目前，base64支持尚未完成（仅支持base64编码的ML和YAML加载，还不支持编码）。现在，存储在FileNode中的序列的随机访问是O(N)时间复杂度; 使用cv::FileNodeIterator可以更快地顺序访问。另一方面，加载FileStorage比以前少了3-6倍的内存！
OpenCV 4.0 现在是基于C++ 11，所以编译OpenCV时需要支持C++ 11 的编译器。请注意，通过将-DENABLE_CXX11 = ON标志传递给CMake，OpenCV 3.x也可以构建为C ++ 11库。 现在这已经变成必选项，因此不需要ON标志。
感谢扩展的C++ 11标准库，我们可以摆脱自己实现的cv::String和cv::Ptr。 现在cv::String == std::string和cv::Ptr是std::shared_ptr之上的thin warpper。 另外，在Linux/BSD for cv :: parallel_for_上，我们现在使用std::thread而不是pthreads。
DNN模块的改进：
增加Mask-RCNN模型的支持。其使用指南：TensorFlow-Object-Detection-API并参考Python示例
集成ONNX解析器（parser）。支持多个主流的分类网络。 支持YOLO 目标检测网络（YOLO的ONNX版本缺少一些提供矩形列表的卷积层）
为了进一步优化DNN模块的性能，引入Intel DLDT。 顺便说一下，Intel DLDT 近期已经开源，参考这份指南可知道如何构建和使用带有DLDT的OpenCV
API修改：默认情况下，blobFromImage方法不会交换Red和Blue通道，也不会对输入图像进行裁剪。这个API的修改已经添加到OpenCV 3.4分支。
处于实验中的Vulkan backen已经添加到不支持OpenCL的平台：https://github.com/opencv/opencv/pull/12703
为OpenCV支持的最主流的深度学习网络添加快捷方式（shotcuts）。 你可以指定模型的别名，以跳过预处理参数甚至模型的路径！比如：
python object_detection.py --model opencv_face_detector.caffemodel --config opencv_face_detector.prototxt --mean 104 177 123 --width 300 --height 300
你也可以输入：

python object_detection.py opencv_fd
修复了在AMD和NVIDIA GPUs上的OpenCL加速。 现在你可以为没有其他环境变量的模型启用DNN_TARGET_OPENCL。请注意，DNN_TARGET_OPENCL_FP16仅在英特尔GPU上进行测试，因此仍需要额外的标志
加了全新的opencv_gapi模块。 它是非常有效的图像处理的引擎，基于处理图的lazy评估和动态构造（因此名称为Graph API或G-API）。 有关详细信息，请参见Graph API（G-API）
性能优化
OpenCV中的几百个基本内核已经使用“广泛通用内在函数（intrinsics）”进行了重写。这些内在函数映射到SSE2，SSE4，AVX2，NEON或VSX内在函数，具体取决于目标平台和编译标志。它应该转化为明显更好的性能，即使对于一些已经优化的功能也是如此。例如，如果使用CPU_BASELINE = AVX2 CMake标志配置和编译OpenCV，则可以为某些图像处理操作提供额外15-30％的速度提升。通过OpenCV 4.0 gold，我们计划将更多内核转换为此类内在函数，并采用我们的动态调度机制，因此在x64平台上，AVX2优化内核始终内置，如果实际硬件支持此类指令，则可以在运行中进行选择（无需更改CPU_BASELINE）。
随着IPPICV组件升级，增加了对IPP 2019的支持
QR 检测器和解码器已经与现有的sample一起添加到opencv/objdetect模块中。目前，解码器构建在QUirc库之上（我们将这个snapshot放入opencv）
主流的Kinect Fusion算法已经实现，针对CPU和GPU（OpenCL）进行优化，并集成到opencv_contrib/rgbd模块中。 为了使现有sample有效，我们在opencv/videoio模块中更新了Kinect 2支持。在OpenCV 4.0 beta版本中，iGPU的代码已经加速，在高分辨率的情况下（512x512x512），性能提升了3倍
非常高效且高质量的DIS密集光流算法已经从opencv_contrib转移到opencv的video 模块。详见示例
同时，较慢的TV L1光流算法已从opencv转移到opencv_contrib
Reference

https://opencv.org/releases.html

https://opencv.org/opencv-4-0-0.html

https://opencv.org/opencv-4-0-0-alpha.html

https://opencv.org/opencv-4-0-0-beta.html

https://opencv.org/opencv-4-0-0-rc.html
