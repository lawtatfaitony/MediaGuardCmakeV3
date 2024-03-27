#pragma once
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <stdio.h>
#include <time.h>
#include <chrono>
#include <regex>

#include "RtspStreamHandle.h"  
#include "interface/CameraMpeg.h"


using namespace Stream;
namespace fs = std::filesystem;
// avformat_open_input/av_read_frame timeout callback
// return: 0(continue original call), other(interrupt original call)
int RtspStreamHandle::read_interrupt_cb(void* pContext)
{
	/*av_log(NULL, AV_LOG_INFO, "func::read_interrupt_cb  \n");
	LOG(INFO) << "[RtspStreamHandel::read_interrupt_cb]";*/
	return 0;
}

enum AVPixelFormat RtspStreamHandle::get_hw_format(AVCodecContext* ctx, const enum AVPixelFormat* pix_fmts)
{
	const enum AVPixelFormat* pPixeFmt;
	for (pPixeFmt = pix_fmts; *pPixeFmt != -1; pPixeFmt++)
	{
		if (*pPixeFmt == ctx->pix_fmt)
			return *pPixeFmt;
	}
	fprintf(stderr, "Failed to get HW surface format.\n");
	return AV_PIX_FMT_NONE;
}


int RtspStreamHandle::hw_decoder_init(AVCodecContext* ctx, const enum AVHWDeviceType type)
{
	int nCode = 0;
	if ((nCode = av_hwdevice_ctx_create(&m_pHDCtx, type, NULL, NULL, 0)) < 0) {
		fprintf(stderr, "Failed to create specified HW device.\n");
		return nCode;
	}
	ctx->hw_device_ctx = av_buffer_ref(m_pHDCtx);
	return nCode;
}

RtspStreamHandle::RtspStreamHandle()
	: m_bExit(false)
	, m_pInputAVFormatCtx(nullptr)
	, m_pOutputFileAVFormatCtx(nullptr)
	, m_pOutputStreamAVFormatCtx(nullptr)
	, m_pHlsOutStreamAVFormatCtx(nullptr)
	, m_bInputInited(false)
	, m_bOutputInited(false)
	, m_bHlsOutInited(false)
	, m_bFirstRun(true)
	, m_pVideoDecoderCtx(nullptr)
	, m_pAudioDecoderCtx(nullptr)
	, m_AVPixelFormat(nullptr) //
	, m_pHDCtx(nullptr)
	, m_nFrame(0)
	, m_pSwsCtx(nullptr)
	, cameraConnectingStatus(CameraConnectingStatus::InPlaying)
	, m_path_filename("")
	
{
	m_poolSavePic.Start(); 
}

RtspStreamHandle::~RtspStreamHandle()
{
	StopDecode();
}

bool RtspStreamHandle::StartDecode(const StreamInfo& infoStream)
{
	m_infoStream = infoStream;

	//判斷 hls/{cameraId}目錄是否存在 
	const fs::path hls_camera_path = fs::current_path() / kHlsDir / std::to_string(m_infoStream.nCameraId);
	if (!File::isDirectoryExists(hls_camera_path.string()))
		File::CreateSingleDirectory(hls_camera_path.string());

	//启动定时清理ts切片，保留最近30秒的 規則改為 只保留index.m3u8包含的文件
	//m_thHlsClear = std::thread(std::bind(&RtspStreamHandle::clean_hls_ts_run, this));
	 
	//------------------------------------------------------------------------------------
	return start_decode();
}

void RtspStreamHandle::StopDecode()
{
	m_bExit = true;
	//如果停止解码 则必须变更链接状态为停止，否则其他个别状态会被视作重新请求重连。
	cameraConnectingStatus = CameraConnectingStatus::InStopped;
	if (m_thDecode.joinable())
		m_thDecode.join();

	if (m_thHlsClear.joinable())
		m_thHlsClear.join();

	if (m_thAddNewRecordInfo.joinable())
		m_thAddNewRecordInfo.join();

	close_input_stream();
	close_output_stream();
	if (m_pHDCtx != nullptr) {
		av_buffer_unref(&m_pHDCtx);
		m_pHDCtx = nullptr;
	}
	m_poolSavePic.Stop();
	sws_freeContext(m_pSwsCtx);
}

void RtspStreamHandle::GetRtmpUrl(std::string& strRtmp)
{
	strRtmp = get_filename(Stream::FType::kFileTypeRtmp);
	return;
}

void RtspStreamHandle::GetVideoSize(int& width, int& height)
{
	width = m_infoStream.nWidth;
	height = m_infoStream.nHeight;
}

void RtspStreamHandle::PushFrame(const cv::Mat& frame)
{
	// save per 25 frame
	m_nFrame++;
	if (0 == round(m_nFrame % m_infoStream.savePictRate)) {
		m_nFrame = 0; //重置计数器
		std::string filename = get_filename(FType::kFileTypePicture);
		//多个线程任务提交到线程池保存磁盘
		m_poolSavePic.Commit([=]()
			{
				cv::imwrite(filename, frame);
			});

		{
			std::lock_guard<std::mutex> lock(m_mtFrame);
			PictInfo pictInfo;
			pictInfo.camera_id = m_infoStream.nCameraId;
			pictInfo.path_filename = filename;
			//TEST
			//LOG(INFO) << "\nfunc::RtspStreamHandle::PushFrame std::string path_filename = filename; path_filename = " << filename;
			int64_t create_time = Time::GetMilliTimestamp();
			int width = m_infoStream.nWidth;
			int height = m_infoStream.nHeight;
			//计算Mat2Base64函数耗时 
			pictInfo.frameBase64 = Basic::CvMatToBase64::Mat2Base64(frame, ".jpg");  //avg of elapsed time = 40-70ms 

			//TEST
			//LOG(INFO) << "\nfunc::RtspStreamHandle::PushFrame path_filename = " << pictInfo.path_filename; 
			m_listFrame.push_back(pictInfo);
		}
	}
}

bool RtspStreamHandle::PopFrame(PictInfo& pictInfo)
{
	std::lock_guard<std::mutex> lock(m_mtFrame);
	if (m_listFrame.empty()) return false;
	pictInfo = m_listFrame.front();
	m_listFrame.pop_front();
	return true;
}

bool RtspStreamHandle::start_decode()
{
	if (!open_input_stream())  //加个while 持续尝试打开
	{
		LOG(ERROR) << "cameraId=" << m_infoStream.nCameraId << " Can't open input:" << m_infoStream.strInput.c_str() << " \nCameraConnectingStatus = " << std::to_string((int)CameraConnectingStatus::InDisConnencted);
		cameraConnectingStatus = CameraConnectingStatus::InDisConnencted;
	}
	else {
		cameraConnectingStatus = CameraConnectingStatus::InPlaying; //成功连接状态
	}

	//如果HLS播放请求 则打开输出流
	if (m_infoStream.nStreamDecodeType == StreamDecodeType::HLS)
		VERIFY_RETURN(open_output_hls_stream(m_pHlsOutStreamAVFormatCtx, m_infoStream.nStreamDecodeType));

	if (m_infoStream.bSaveVideo)
		VERIFY_RETURN(open_output_stream(m_pOutputFileAVFormatCtx));

	m_thDecode = std::thread(std::bind(&RtspStreamHandle::do_decode, this));
	return true;
}

bool RtspStreamHandle::open_input_stream()
{
	if (m_pInputAVFormatCtx)
	{
		std::string strError = "avformat already exists";
		return false;
	}
	AVDictionary* pOption = NULL;
	m_pInputAVFormatCtx = avformat_alloc_context();
	m_pInputAVFormatCtx->flags |= AVFMT_FLAG_NONBLOCK;

	av_dict_set(&pOption, "rtsp_transport", "tcp", 0);

#ifdef _WIN32
	av_dict_set(&pOption, "buffer_size", "409600", 0);
	av_dict_set(&pOption, "probesize", "1048576", 0);	 //加大读取区的缓存
#endif 

#ifdef __linux__
	av_dict_set(&pOption, "buffer_size", "204800", 0); // 降低 buffer_size 的大小為 200KB
	av_dict_set(&pOption, "probesize", "524288", 0); // 降低 probesize 的大小為 512KB
#endif 
	 
	av_dict_set(&pOption, "stimeout", "2000000", 0);	 //如果没有设置stimeout，那么把ipc网线拔掉，av_read_frame会阻塞（时间单位是微妙）		
	av_dict_set(&pOption, "fflags", "nobuffer", 0);		 //无缓存，解码时有效 https://blog.csdn.net/asdasfdgdhh/article/details/125501488
	
	m_pInputAVFormatCtx->interrupt_callback = { read_interrupt_cb, this };

	int nCode = avformat_open_input(&m_pInputAVFormatCtx, m_infoStream.strInput.c_str(), nullptr, &pOption);
	if (nCode < 0)
	{
		std::string strError = "CameraId=" + std::to_string(m_infoStream.nCameraId) + " Can't open input:" + m_infoStream.strInput + "(err:avformat_open_input)" + get_error_msg(nCode);
		return false;
	}
	std::cout << "\nCameraId=" + std::to_string(m_infoStream.nCameraId) << " Open input[" << m_infoStream.strInput << "] success.\n";

	// retrieve stream information 获取音视频信息
	if (avformat_find_stream_info(m_pInputAVFormatCtx, 0) < 0)
	{
		std::string strError = "Can't find stream info";
		return false;
	}
	//display pFormatCtx->streams 把音视频信息打印出来
	std::cout << "\n-------------------------- device infomation --------------------------\n" << std::endl;
	std::cout << "--------------------------" << " CameraId=" + std::to_string(m_infoStream.nCameraId) << " --------------------------\n" << std::endl;

	av_dump_format(m_pInputAVFormatCtx, 0, m_infoStream.strInput.c_str(), 0);

	// open codec context for video
	if (open_codec_context(m_infoStream.nVideoIndex, &m_pVideoDecoderCtx, m_pInputAVFormatCtx, AVMEDIA_TYPE_VIDEO)) {
		m_infoStream.nWidth = m_pVideoDecoderCtx->width;
		m_infoStream.nHeight = m_pVideoDecoderCtx->height;
		m_infoStream.nPixFmt = m_pVideoDecoderCtx->pix_fmt;
	}
	else
	{
		std::cout << "CameraId=" << std::to_string(m_infoStream.nCameraId) << " Open codec context failed\n" << std::endl;
		return false;
	}
	// 返回音频解码上下文 与 输入格式 (open_codec_context函数先写好才能如下使用)
	//open codec context for audio
	bool audio_decoder_succ = open_codec_context(m_infoStream.nAudioIndex, &m_pAudioDecoderCtx, m_pInputAVFormatCtx, AVMEDIA_TYPE_AUDIO);
	if (!audio_decoder_succ && kInvalidStreamIndex == m_infoStream.nAudioIndex) // && kInvalidStreamIndex == m_infoStream.nAudioIndex kInvalidStreamIndex== m_infoStream.nAudioIndex   kInvalidStreamIndex初始化等于-1表示初始没有
	{
		std::string strError = "\nCameraId=" + std::to_string(m_infoStream.nCameraId) + " Can't find audio stream in the rtsp input\n";
		std::cout << strError.c_str() << std::endl;
		//return false;  //导致停止；没有音频就PASS
	}

	return true;
}

/* 
*參考技術文庫 
* https://ffmpeg.xianwaizhiyin.net/api-ffmpeg/decode.html
*/
bool RtspStreamHandle::open_codec_context(int& nStreamIndex, AVCodecContext** pDecoderCtx, AVFormatContext* pFmtCtx, enum AVMediaType nMediaType)
{
	AVStream* pStream = nullptr;
	AVCodec* pDecoder = nullptr;
	AVDictionary* pOptions = nullptr;
 
#ifdef _WIN32
	int stream_index = av_find_best_stream(pFmtCtx, nMediaType, -1, -1, (AVCodec**)&pDecoder, 0);  //據說可以 把參數 AVCodec **decoder_ret 傳入 null
#endif 

#ifdef __linux__
	int stream_index = av_find_best_stream(pFmtCtx, nMediaType, -1, -1, (const AVCodec**)&pDecoder, 0);
#endif 
	

	if (stream_index < 0)
	{
		fprintf(stderr, "Couldn't find %s stream in input\n", av_get_media_type_string(nMediaType));
		return false;
	}
	nStreamIndex = stream_index;
	AVPixelFormat nPixeFmt = AV_PIX_FMT_NONE;

	if (AVMEDIA_TYPE_VIDEO == nMediaType && m_infoStream.nHDType > AV_HWDEVICE_TYPE_NONE)
	{
		// get hard device
		for (int i = 0;; i++)
		{
			try {
				const AVCodecHWConfig* pConfig = avcodec_get_hw_config(pDecoder, i);
				if (nullptr == pConfig) continue;
				if (!pConfig)
				{
					fprintf(stderr, "Decoder %s does not support device type %s.\n",
						pDecoder->name, av_hwdevice_get_type_name((AVHWDeviceType)m_infoStream.nHDType)); 
					  //打印解码器类型
					av_log(NULL, AV_LOG_INFO, "Decoder %s does support device type %s.\n", pDecoder->name, av_hwdevice_get_type_name((AVHWDeviceType)m_infoStream.nHDType));
				    LOG(INFO) << "\nDecoder " << pDecoder->name << " does support device type" << "av_hwdevice_get_type_name = "<< av_hwdevice_get_type_name((AVHWDeviceType)m_infoStream.nHDType)<<"\n";
					return false;
				}
				if (pConfig->methods & AV_CODEC_HW_CONFIG_METHOD_HW_DEVICE_CTX &&
					pConfig->device_type == m_infoStream.nHDType) {
					nPixeFmt = pConfig->pix_fmt;

					fprintf(stderr, "Decoder %s does support device type %s.\n", pDecoder->name, av_hwdevice_get_type_name((AVHWDeviceType)m_infoStream.nHDType));  //打印解码器类型
					av_log(NULL, AV_LOG_INFO, "Decoder %s does support device type %s.\n", pDecoder->name, av_hwdevice_get_type_name((AVHWDeviceType)m_infoStream.nHDType));
					break;
				}
			}
			catch (...)
			{
				fprintf(stderr, "hw_decoder_init fail %s \n", av_get_media_type_string(nMediaType));
				av_log(NULL, AV_LOG_INFO, "hw_decoder_init fail %s \n", av_get_media_type_string(nMediaType));
			}
		}
	}
	/* Allocate a codec context for the decoder */
	*pDecoderCtx = avcodec_alloc_context3(pDecoder);
	if (!*pDecoderCtx)
	{
		fprintf(stderr, "Failed to allocate the %s codec context\n",
			av_get_media_type_string(nMediaType));
		return false;
	}

	/* Copy codec parameters from input stream to output codec context */
	pStream = m_pInputAVFormatCtx->streams[nStreamIndex];
	if ((stream_index = avcodec_parameters_to_context(*pDecoderCtx, pStream->codecpar)) < 0)
	{
		fprintf(stderr, "Failed to copy %s codec parameters to decoder context\n", av_get_media_type_string(nMediaType));
		return false;
	}
	// init the hard device decoder
	if (AVMEDIA_TYPE_VIDEO == nMediaType && nPixeFmt != AV_PIX_FMT_NONE)
	{
		//沒有音頻的情況出現嚴重錯誤
		try {
			(*pDecoderCtx)->get_format = get_hw_format;
			(*pDecoderCtx)->pix_fmt = nPixeFmt;
			hw_decoder_init(*pDecoderCtx, (AVHWDeviceType)m_infoStream.nHDType);
			fprintf(stderr, "hw_decoder_init success %s \n", av_get_media_type_string(nMediaType));
			av_log(NULL, AV_LOG_INFO, "hw_decoder_init success %s \n", av_get_media_type_string(nMediaType));
		}
		catch (...)
		{
			fprintf(stderr, "hw_decoder_init fail!!! %s \n", av_get_media_type_string(nMediaType));
			av_log(NULL, AV_LOG_INFO, "hw_decoder_init success %s \n", av_get_media_type_string(nMediaType));
		} 
	}
	else {
		fprintf(stderr, "init the hard device decoder fail!! : %s \n", av_get_media_type_string(nMediaType));
	}

	//初始化音频 的硬件解码 以下的没法测试 ,补充于2023年1月28日
	if (AVMEDIA_TYPE_AUDIO == nMediaType && nPixeFmt != AV_PIX_FMT_NONE && m_infoStream.nAudioIndex != -1)
	{
		try {
			(*pDecoderCtx)->get_format = get_hw_format;
			(*pDecoderCtx)->pix_fmt = nPixeFmt;
			hw_decoder_init(*pDecoderCtx, (AVHWDeviceType)m_infoStream.nHDType); //HWDeviceType
		}
		catch (...)
		{
			fprintf(stderr, "audio decode hw_decoder_init fail!!! %s \n", av_get_media_type_string(nMediaType));
			av_log(NULL, AV_LOG_INFO, "audio decode hw_decoder_init fail %s \n", av_get_media_type_string(nMediaType));
		}
	}
	else {
		fprintf(stderr, "No AVMEDIA_TYPE_AUDIO (line 351): %s \n", av_get_media_type_string(nMediaType));
	}

	/* Init the decoders, with or without reference counting */
	av_dict_set(&pOptions, "refcounted_frames", m_infoStream.nRefCount ? "1" : "0", 0);
	if ((stream_index = avcodec_open2(*pDecoderCtx, pDecoder, &pOptions)) < 0)
	{
		fprintf(stderr, "Failed to open %s codec\n",
			av_get_media_type_string(nMediaType));
		return false;
	}
	return true;
}

void RtspStreamHandle::close_input_stream()
{
	if (m_pInputAVFormatCtx)
		avformat_close_input(&m_pInputAVFormatCtx);

	if (m_pVideoDecoderCtx)
		avcodec_free_context(&m_pVideoDecoderCtx);

	if (m_pAudioDecoderCtx)
		avcodec_free_context(&m_pAudioDecoderCtx);

	if (m_pHlsOutStreamAVFormatCtx)
		avformat_close_input(&m_pHlsOutStreamAVFormatCtx);
}

bool RtspStreamHandle::open_output_stream(AVFormatContext*& pFormatCtx, bool bRtmp)
{
	if (pFormatCtx)
	{
		printf("Already has output avformat\n");
		return false;
	}
	if (m_infoStream.strOutput.empty())
	{
		printf("Invalid output file\n");
		return false;
	}
	std::string strFormatName = bRtmp ? "flv" : "mp4";
	std::string strOutputPath = bRtmp ? m_infoStream.strOutput : get_filename(FType::kFileTypeVideo);
	m_path_filename = strOutputPath;
	int nCode = avformat_alloc_output_context2(&pFormatCtx, NULL, strFormatName.c_str(), strOutputPath.c_str());
	if (nullptr == pFormatCtx)
	{
		printf("Can't alloc output context (RtspStreamHandle::open_output_stream::avformat_alloc_output_context2) \n");
		return false;
	}
	 
	for (auto nIndex = 0; nIndex < m_pInputAVFormatCtx->nb_streams; ++nIndex)
	{
		AVStream* pInStream = m_pInputAVFormatCtx->streams[nIndex];
		AVStream* pOutStream = avformat_new_stream(pFormatCtx, nullptr);
		if (!pOutStream)
		{
			printf("Can't new out stream");
			return false;
		}
		pOutStream->codecpar->codec_type = pInStream->codecpar->codec_type;

		//copy the encode info to output
		nCode = avcodec_parameters_copy(pOutStream->codecpar, pInStream->codecpar);
		if (nCode < 0)
		{
			std::string strError = "Can't copy context, url: " + m_infoStream.strInput + ",errcode:"
				+ std::to_string(nCode) + ",err msg:" + get_error_msg(nCode);
			printf("%s \n", strError.c_str());
			return false;
		}
		// 标记不需要重新编解码
		pOutStream->codecpar->codec_tag = 0;
	}
	av_dump_format(pFormatCtx, 0, strOutputPath.c_str(), 1);

	if (!(pFormatCtx->oformat->flags & AVFMT_NOFILE))
	{
		nCode = avio_open2(&pFormatCtx->pb, strOutputPath.c_str(), AVIO_FLAG_WRITE, nullptr, nullptr);
		if (nCode < 0)
		{
			std::string strError = "Can't open output io, file:" + strOutputPath + ",errcode:" + std::to_string(nCode) + ", err msg:"
				+ get_error_msg(nCode);
			printf("%s \n", strError.c_str());
			return false;
		}
	}
	//写文件头
	nCode = avformat_write_header(pFormatCtx, NULL);
	if (nCode < 0)
	{
		std::string strError = "Can't write outputstream header, URL:" + strOutputPath + ",errcode:" + std::to_string(nCode) + ", err msg:"
			+ get_error_msg(nCode);
		printf("%s \n", strError.c_str());
		m_bOutputInited = false;
		return false;
	}
	m_bOutputInited = true;
	return true;
}

bool RtspStreamHandle::open_output_hls_stream(AVFormatContext*& pFormatCtx, int nStreamDecodeType)   
{
	StreamDecodeType streamDecodeType = (StreamDecodeType)nStreamDecodeType;

	if (pFormatCtx)
	{
		printf("Already has output avformat \n");
		return false;
	}

	if (m_infoStream.strOutput.empty())
	{
		printf("Invalid output file\n");
		return false;
	}
	if (streamDecodeType != StreamDecodeType::NOSTREAM)
	{
		if (streamDecodeType == StreamDecodeType::HLS)
			m_infoStream.strOutput = get_filename(FType::kFileTypeHls);
		else
			m_infoStream.strOutput = get_filename(FType::kFileTypeRtmp);
	}
	std::string strFormatName = "";

	if (streamDecodeType == StreamDecodeType::HLS)
		strFormatName = "hls";

	//返回index.m3u8的路徑  test ok
	std::string strOutputPath = get_filename(FType::kFileTypeHls);  
	int nCode = avformat_alloc_output_context2(&pFormatCtx, NULL, strFormatName.c_str(), strOutputPath.c_str());
	 
	if (nullptr == pFormatCtx)
	{
		LOG(WARNING) << "[hls] RtspStramHandle::avformat_alloc_output_context2:" << strOutputPath;
		printf("Can't alloc hls output context %s\n", strOutputPath.c_str());
		return false;
	}
	 
	for (auto nIndex = 0; nIndex < m_pInputAVFormatCtx->nb_streams; ++nIndex)
	{
		AVStream* pInStream = m_pInputAVFormatCtx->streams[nIndex];
		AVStream* pOutStream = avformat_new_stream(pFormatCtx, nullptr);
		if (!pOutStream)
		{
			printf("Can't new out stream for hls");
			return false;
		}

		//这句复制解码参数从输入文法，不知道有什么用的
		pOutStream->codecpar->codec_type = pInStream->codecpar->codec_type;

		//copy the encode info to output
		nCode = avcodec_parameters_copy(pOutStream->codecpar, pInStream->codecpar);
		if (nCode < 0)
		{
			std::string strError = "[func::open_output_hls_stream] Can't copy context, url: " + m_infoStream.strInput + ",errcode:"
				+ std::to_string(nCode) + ",err msg:" + get_error_msg(nCode);
			printf("%s \n", strError.c_str());
			LOG(ERROR) << strError;
			return false;
		}
		// 标记不需要重新编解码
		//pOutStream->codecpar->codec_tag = 0;
	}
	av_dump_format(pFormatCtx, 0, strOutputPath.c_str(), 1);

	if (!(pFormatCtx->oformat->flags & AVFMT_NOFILE))
	{
		//avio_open 实例 https://vimsky.com/examples/detail/cpp-ex-----avio_open2-function.html
		nCode = avio_open(&pFormatCtx->pb, strOutputPath.c_str(), AVIO_FLAG_WRITE);
		if (nCode < 0)
		{
			std::string strError = "Can't open output io, file:" + strOutputPath + ",errcode:" + std::to_string(nCode) + ", err msg:"
				+ get_error_msg(nCode) + "/t line 552";
			printf("%s \n", strError.c_str());
			return false;
		}
		
	}
	 
	if (streamDecodeType == StreamDecodeType::HLS)
	{
		/*
		 * 参数参考 https://www.cnblogs.com/michong2022/p/17016423.html
		*/
		av_opt_set(pFormatCtx->priv_data, "is_live", "true", AV_OPT_SEARCH_CHILDREN);      //是否直播 出错
		av_opt_set(pFormatCtx->priv_data, "hls_list_size", "8", AV_OPT_SEARCH_CHILDREN);   //设置m3u8文件播放列表保存的最多条目，设置为0会保存有所片信息，默认值为5
		av_opt_set(pFormatCtx->priv_data, "hls_wrap", "8", AV_OPT_SEARCH_CHILDREN);
		av_opt_set(pFormatCtx->priv_data, "hls_time", "6", AV_OPT_SEARCH_CHILDREN);        //默认2seconds 
		av_opt_set(pFormatCtx->priv_data, "hls_flags", "0", AV_OPT_SEARCH_CHILDREN);
		//---------------------------------------------------------------------------------- 
#ifdef DEBUG
		LOG(INFO) << "StreamDecodeType::HLS and av_opt_set params hls_list_size = 8 hls_time = 6's \n" << strOutputPath;
#endif // DEBUG
	}
	//写文件头
	nCode = avformat_write_header(pFormatCtx, NULL);
	  
	if (nCode < 0)
	{
		std::string strError = "Can't write hls output stream header, URL:" + strOutputPath + ",errcode:" + std::to_string(nCode) + ", err msg:"
			+ get_error_msg(nCode);
		printf("%s \n", strError.c_str());
		m_bOutputInited = false;
		return false;
	}
	m_bOutputInited = true; //是否有需要在这里保留
	return true;
}

void RtspStreamHandle::write_output_hls_stream(AVFormatContext* pFormatCtx, const AVPacket& packet)
{
	// check inited
	if (!m_bOutputInited || nullptr == pFormatCtx) {
		return;
	}
	// check the packet
	if (nullptr == packet.buf || 0 == packet.buf->size) {
		return;
	}

	AVPacket pktFrame ;
	av_init_packet(&pktFrame);
	av_packet_ref(&pktFrame, &packet);

	AVStream* pInStream = m_pInputAVFormatCtx->streams[pktFrame.stream_index];
	AVStream* pOutStream = pFormatCtx->streams[pktFrame.stream_index];

	av_packet_rescale_hls_ts(&pktFrame, pInStream->time_base, pOutStream->time_base);

	int nError = av_interleaved_write_frame(pFormatCtx, &pktFrame);
	 
	if (nError != 0)
	{
		printf("Error: %d while write_output_hls_stream write packet frame, %s\n", nError, get_error_msg(nError).c_str());
		return;
	}
	 
	return;
}

void RtspStreamHandle::av_packet_rescale_hls_ts(AVPacket* pkt, AVRational src_tb, AVRational dst_tb)
{
	if (pkt->pts != AV_NOPTS_VALUE)
		pkt->pts = av_rescale_q(pkt->pts, src_tb, dst_tb);
	if (pkt->dts != AV_NOPTS_VALUE)
		pkt->dts = av_rescale_q(pkt->dts, src_tb, dst_tb);
	if (pkt->duration > 0)
		pkt->duration = av_rescale_q(pkt->duration, src_tb, dst_tb);
}

//void RtspStreamHandle::clean_hls_ts_run()
//{
//	if (m_infoStream.nStreamDecodeType != StreamDecodeType::HLS)
//	{
//		 
//		//必须是HLS流请求才继续执行ts切片清理
//		return;
//	}
//	while (!m_bExit.load())
//	{ 
//		clean_hls_ts(30); //保留20秒的文件记录 注意设置hls av_dist的时候不能大于这个时间，
//		SHARED_LOCK(m_mtLock);  // std::unique_lock<std::mutex> locker(lock);
//		m_cvCond.wait_for(locker, std::chrono::milliseconds(60000), [this]() {  //60's
//			auto isExit = m_bExit.load();
//			return isExit;
//		});
//	}
//}
  
//void RtspStreamHandle::clean_hls_ts(int tsRemainSeconds)
//{
//	const fs::path hls_camera_path = fs::current_path() / kHlsDir / std::to_string(m_infoStream.nCameraId);
//	const fs::path hls_path__filename_index_m3u8 = hls_camera_path / "index.m3u8";
//	 
//	std::vector<std::string> vecFile;
//
//	File::GetFilesOfDir(hls_camera_path.string(), vecFile);
//
//	for (size_t i = 0; i < vecFile.size(); i++) {
//		 
//		int iCreateTime, iModifyTime, iAccessTime, iFileLen;
//
//		const std::string ts_path_filename = vecFile[i].c_str();
//
//		if (true == File::get_file_info(ts_path_filename, iCreateTime, iModifyTime, iAccessTime, iFileLen))
//		{
//			//获取多少分钟前的时间
//			std::chrono::system_clock::time_point curr = std::chrono::system_clock::now();
//			std::chrono::system_clock::time_point before_minutes_time = curr - std::chrono::seconds(tsRemainSeconds);
//			auto longremaintime = std::chrono::duration_cast<std::chrono::seconds>(before_minutes_time.time_since_epoch());
//			int64_t longCreateTime = static_cast<int64_t>(iCreateTime);
//			if (longCreateTime < longremaintime.count())
//			{ 
//				try {
//
//					File::deleteFile(ts_path_filename);
//				}
//				catch (const std::exception& ex) {
//					LOG(ERROR) << ts_path_filename << " [EXCEPTION] DELETED TS FILE FAIL: " << ex.what() << "\n" << ts_path_filename << std::endl;
//				}
//				catch (...) {
//					LOG(ERROR) << ts_path_filename << " [EXCEPTION] DELETED TS FILE FAIL: Unknown exception" << std::endl;
//				}
//
//			}
//			//for TEST
//			else {
//				LOG(INFO) << Time::GetCurrentSystemTime() << "[file not in " << tsRemainSeconds << " seconds ago]" << " file create:" << longCreateTime << "-" << iCreateTime << " | " << longremaintime.count();
//			}
//		}
//	}
//	vecFile.clear();
//}


void RtspStreamHandle::close_output_stream()
{
	release_output_format_context(m_pOutputFileAVFormatCtx);
	release_output_format_context(m_pOutputStreamAVFormatCtx);
	release_output_format_context(m_pHlsOutStreamAVFormatCtx);
	m_bOutputInited = false;
	m_bHlsOutInited = false;
}

void RtspStreamHandle::do_decode()
{
	int64_t nFrame = 0;
	AVPacket packet;
	int64_t nLastSaveVideo = Time::GetMilliTimestamp();
	int64_t nVideoTime = m_infoStream.nVideoTime;

	//开始录像时间
	start_time = av_gettime();
	LOG(INFO) << "\ninitializing cameraId=" << m_infoStream.nCameraId << " start_time=" << start_time << "\n" << std::endl;
	cameraConnectingStatus = CameraConnectingStatus::InPlaying; //改变 状态为Inplaying
	while (!m_bExit)
	{
		int nCode = 0;
		if (nCode = av_read_frame(m_pInputAVFormatCtx, &packet), nCode < 0)
		{
			LOG(ERROR) << "Read frame failed," << get_error_msg(nCode).c_str();

			//读帧失败，改变当前连接状态为 InDisConnencted（断开状态中）
			cameraConnectingStatus = CameraConnectingStatus::InDisConnencted;

			break; //中断读取帧 禁止不循环下去改为continue 重新拉流
		}
		bool bSucc = false;
		if (m_infoStream.nVideoIndex == packet.stream_index)
			bSucc = decode_video_packet(&packet);
		else if (packet.stream_index == m_infoStream.nAudioIndex)
			bSucc = decode_audio_packet(packet);

		if (!bSucc) continue;

		if (m_infoStream.bSaveVideo)
		{
			// save video with slice
			// it needs optimization, save in thread //下一步改為使用線程優化
			save_stream(m_pOutputFileAVFormatCtx, packet);

			int64_t  nMillSecond = Time::GetMilliTimestamp();

			if (nMillSecond - nLastSaveVideo >= nVideoTime)
			{
				//錄像時間結束，釋放輸出文法
				release_output_format_context(m_pOutputFileAVFormatCtx);

				//记录录像信息到云端
				if (m_thAddNewRecordInfo.joinable())
					m_thAddNewRecordInfo.join();

				m_thAddNewRecordInfo = std::thread(std::bind(&RtspStreamHandle::addnew_record_file_info, this, m_infoStream, nLastSaveVideo, nMillSecond));

				//重置开始StartTimeStamp
				nLastSaveVideo = nMillSecond;

				//重置第一幀
				recfirstPTS = 0;
				//重新打開輸出流
				open_output_stream(m_pOutputFileAVFormatCtx);
			}
		}

		//hls解码  
		if (m_infoStream.nStreamDecodeType == StreamDecodeType::HLS)
			write_output_hls_stream(m_pHlsOutStreamAVFormatCtx, packet);

		//===============================================
		if (m_infoStream.nStreamDecodeType == StreamDecodeType::RTMP) {
			printf("[fun::do_decode] m_infoStream.bRtmp = true \n");
			save_stream(m_pOutputStreamAVFormatCtx, packet);
		}

		/*
		av_read_frame 会产生 6MB 的堆内存。如果不进行 av_packet_unref，则会导致内存泄漏。即使是同一个栈变量 pkt，即使出了这个栈变量的作用范围、这个栈变量被系统收回，那些每次产生的 6MB 堆内存们，也不会被收回。
		泄漏的原因在于，当 ·av_read_frame() < 0 时，它也产生了堆内存，也需要用 av_packet_unref 进行释放；当 pkt.stream_idx != streamidx 时，也需要用 av_packet_unref 进行释放。
		*/
		av_packet_unref(&packet);
		++nFrame;
	}
	//printf("Reading ended,CameraId=%s rea%10d %s video frames ", to_string(m_infoStream.nCameraId), to_string(nFrame));  //printf ("Decimals: %d %ld\n", 1977, 650000L);
	printf("\nReading ended,CameraId=%i read %llu\n", m_infoStream.nCameraId, nFrame);
	printf("\n------------------------------------------------------------------------------\n");
}

//队列处理
void RtspStreamHandle::push_packet(const AVPacket& packet)
{
	/*
	  這樣的寫法 參考 C++ 的 Rii 編程思想
	  REF : https://www.cnblogs.com/jiangbin/p/6986511.html
	  使用std::unique_lock或者std::lock_guard对互斥量进行状态管理：
	  在创建std::lock_guard对象的时候，会对std::mutex对象进行lock，当std::lock_guard对象在超出作用域时，会自动std::mutex对象进行解锁，这样的话，就不用担心代码异常造成的线程死锁。
	*/

	{
		std::lock_guard<std::mutex> lock(m_mtPacket);
		m_listPacket.push_back(packet);
	}

	/*
	std::condition_variable （m_cvFrame）
	std::condition_variable，是C++11提供的条件变量，可用于同时阻塞一个线程或多个线程。一般的，生产者线程利用支持std::mutex的std::lock_guard/std::unique_lock修改共享变量后，并通知condition_variable。
	消费者线程获取同一个std::mutex(std::unique_lock所持有)，并调用std::condition_variable的wait, wait_for, or wait_until。wait操作会释放互斥量，同时挂起该线程。
	当条件变量收到通知、超时到期或发生虚假唤醒时，线程被唤醒，互斥量也被原始地重新获取。需要注意的是，如果是虚假唤醒，线程应该检查条件并继续等待，以保证业务的正确性。ref:  https://blog.csdn.net/heusunduo88/article/details/124830850. 
	*/
	m_cvFrame.notify_one();
}

bool RtspStreamHandle::decode_video_packet(AVPacket* packet)
{
	int nCode = avcodec_send_packet(m_pVideoDecoderCtx, packet);
	if (nCode < 0) {
		fprintf(stderr, "Error during decoding,%s\n", get_error_msg(nCode).c_str());
		return false;
	}
	AVFrame* pTmpFrame = nullptr;
	AVFrame* pFrame = av_frame_alloc();
	AVFrame* pSwapFrame = av_frame_alloc();
	bool succ = false;
	while (1) {
		if (nullptr == pFrame || nullptr == pSwapFrame)
		{
			fprintf(stderr, "Can't alloc frame\n");
			nCode = AVERROR(ENOMEM);
			break;
		}
		nCode = avcodec_receive_frame(m_pVideoDecoderCtx, pFrame);
		if (nCode == AVERROR(EAGAIN) || nCode == AVERROR_EOF) {
			av_frame_free(&pFrame);
			av_frame_free(&pSwapFrame);
			succ = true;
			break;
		}
		else if (nCode < 0)
		{
			fprintf(stderr, "Error while decoding,%s\n", get_error_msg(nCode).c_str());
			break;
		}

		if (m_infoStream.nPixFmt > AV_HWDEVICE_TYPE_NONE && m_infoStream.nHDType != AV_HWDEVICE_TYPE_NONE
			&& pFrame->format == m_infoStream.nPixFmt)
		{
			/* retrieve data from GPU to CPU */
			if ((nCode = av_hwframe_transfer_data(pSwapFrame, pFrame, 0)) < 0) {
				fprintf(stderr, "Error transferring the data to system memory [av_hwframe_transfer_data]\n");
				succ = false;
				break;
			}
			pTmpFrame = pSwapFrame;
		}
		else {
			pTmpFrame = pFrame;
		}
		//保存图片
		if (m_infoStream.bSavePic)
		{
			PushFrame(avframe_to_mat(pTmpFrame));
		}

		av_frame_unref(pFrame);
		av_frame_unref(pSwapFrame);
	}
	av_frame_free(&pFrame);
	av_frame_free(&pSwapFrame);
	av_frame_unref(pTmpFrame);
	return succ;
}

/// <summary>
///  音频解码
/// </summary>
/// <param name="packet"></param>
/// <returns></returns>
bool RtspStreamHandle::decode_audio_packet(const AVPacket& packet)
{
	int data_size = 0;

	/* send the packet with the compressed data to the decoder */
	int nCode = avcodec_send_packet(m_pAudioDecoderCtx, &packet);
	if (nCode < 0) {
		fprintf(stderr, "Error submitting the packet to the decoder\n");
		return false;
	}
	bool succ = true;
	AVFrame* pFrame = av_frame_alloc();
	/* read all the output frames (in general there may be any number of them */
	while (nCode >= 0) {
		nCode = avcodec_receive_frame(m_pAudioDecoderCtx, pFrame);
		if (AVERROR(EAGAIN) == nCode || AVERROR_EOF == nCode) {

		}
		else if (nCode < 0) {
			fprintf(stderr, "Error during decoding\n");
			succ = false;
		}
		//这段是音频解码保存 去掉注释进行测试 2023-2-10
		/*
		参考视音频同步 实现 代码:https://github.com/brookicv/FSplayer
		ref 解析文章 https://blog.csdn.net/qq_40170041/article/details/128233616
		*/
		//data_size = av_get_bytes_per_sample(m_pAudioDecoderCtx->sample_fmt);
		//if (data_size < 0) {
		//	/* This should not occur, checking just for paranoia */
		//	fprintf(stderr, "Failed to calculate data size\n");
		//	return false;
		//}
		//for (int i = 0; i < pFrame->nb_samples; i++) {
		//	for (int nChl = 0; nChl < m_pAudioDecoderCtx->channels; nChl++) {
		//		fwrite(pFrame->data[nChl] + data_size * i, 1, data_size, output_file); //outfile
		//	}
		//}

		//ref http://cn.voidcc.com/question/p-gjfuvrev-du.html
	}

	av_frame_free(&pFrame);
	return succ;
}

void RtspStreamHandle::save_stream(AVFormatContext* pFormatCtx, const AVPacket& packet)
{
	//compute_muxer_pkt_fields  这个函数是否需要使用的？？

	// check inited
	if (!m_bOutputInited || nullptr == pFormatCtx) {
		return;
	}
	// check the packet
	if (nullptr == packet.buf || 0 == packet.buf->size) {
		return;
	}
	/*
		pts: 演播时间戳
		dts:解码时间戳
		duration : 播放时长
	*/
	AVPacket pktFrame /*= packet*/;
	av_init_packet(&pktFrame);
	av_packet_ref(&pktFrame, &packet);
	AVStream* pInStream = m_pInputAVFormatCtx->streams[pktFrame.stream_index];
	AVStream* pOutStream = pFormatCtx->streams[pktFrame.stream_index];

	/*
		 copy packet 转换PTS/DTS（Convert PTS/DTS）
		 參考雷神寫的 https://blog.csdn.net/weixin_43360707/article/details/116736181
	*/
	try {

		//重新调整PTS DTS DUARIONT 方式I
		av_packet_rescale_ts(&pktFrame, pInStream->time_base, pOutStream->time_base, pInStream->r_frame_rate);

	}
	catch (const std::exception& e)
	{
		std::cout << "Convert dts/pts exception:" << e.what() << "\n";
		return;
	}
	catch (...)
	{
		printf("Unkonw error\n");
		return;
	}
	switch (pInStream->codecpar->codec_type)
	{
	case AVMEDIA_TYPE_AUDIO:  //参考这里 看看  https://www.cnblogs.com/gongluck/archive/2019/05/07/10827950.html
	case AVMEDIA_TYPE_VIDEO:
	{
		//LOG(INFO) << "pktFrame.duration =" << pktFrame.duration << "pktFrame.pts=" << pktFrame.pts << "pktFrame.dts=" << pktFrame.dts;
		int nError = av_interleaved_write_frame(pFormatCtx, &pktFrame);
		if (nError != 0)
		{
			printf("Error: %d while writing frame, %s\n", nError, get_error_msg(nError).c_str());
		}
		break;
	}
	default:
		break;
	}
}

void RtspStreamHandle::av_packet_rescale_ts(AVPacket* pkt, AVRational src_tb, AVRational dst_tb, AVRational r_frame_rate)
{
	if (pkt->duration < 0)
	{
		LOG(INFO) << "if (pkt->duration < 0) ：pktFrame->duration =" << pkt->duration << " pktFrame->pts=" << pkt->pts << " pktFrame->dts=" << pkt->dts;
	}

	if (pkt->pts != AV_NOPTS_VALUE)
		pkt->pts = av_rescale_q(pkt->pts, src_tb, dst_tb) - recfirstPTS;

	if (pkt->dts != AV_NOPTS_VALUE)
		pkt->dts = av_rescale_q(pkt->dts, src_tb, dst_tb) - recfirstPTS;

	if (recfirstPTS == 0)
	{
		if (pkt->pts != AV_NOPTS_VALUE) //加这句防止 pts has no value
			recfirstPTS = pkt->pts; //记录每次重新录像的第一帧

		if (pkt->pts != AV_NOPTS_VALUE)
			pkt->pts = 0;

		if (pkt->dts != AV_NOPTS_VALUE)
			pkt->dts = 0; //我会从i帧开始，正常来说i帧的pts和dts是一样的

	}
	if (pkt->duration > 0)
		pkt->duration = av_rescale_q(pkt->duration, src_tb, dst_tb);

}

void RtspStreamHandle::release_output_format_context(AVFormatContext*& pFmtContext)
{
	if (pFmtContext)
	{
		av_write_trailer(pFmtContext);

		start_time = av_gettime(); //重新记录一个新的开始时间

		LOG(INFO) << "Finished file " << m_infoStream.mediaFormate << " - " << m_path_filename << ",if again then record start_time =" << start_time << "\n";

		if (!(pFmtContext->oformat->flags & AVFMT_NOFILE))
		{
			if (pFmtContext->pb)
			{
				avio_close(pFmtContext->pb);
			}
		}
		avformat_free_context(pFmtContext);
		pFmtContext = nullptr;
	}
}

/*
获取MP4/FLV相对路径 VIDEO FILE:video/2023-02-22/1677049165025.flv
包括 RTMP 或 HLS的路径
*/
std::string RtspStreamHandle::get_filename(Stream::FType ftype)
{
	std::string strToday = Time::GetCurrentDate();
	if (strToday != m_strToday)
		m_strToday = strToday;
	std::string strTimestamp = std::to_string(Time::GetMilliTimestamp());
	  
	const std::string video_filename = strTimestamp + (m_infoStream.mediaFormate == FLV ? kVidoeFormatFlv : kVidoeFormat);
	const  fs::path video_path_filename = fs::current_path() / kVideoDir / strToday / video_filename;

	const std::string audio_filename = strTimestamp + ".acc";
	const fs::path audio_path_filename = fs::current_path() / kAudioDir / Time::GetCurrentDate() / audio_filename;

	const std::string picture_filename = strTimestamp + kPictureFormat;
	const fs::path picture_path_filename = fs::current_path() / kPictureDir / picture_filename;
	 
	const fs::path hls_path_filename = fs::current_path() / kHlsDir / std::to_string(m_infoStream.nCameraId) / "index.m3u8";

	switch (ftype)
	{
		case Stream::FType::kFileTypePicture:
			 
			return picture_path_filename.string();

		case Stream::FType::kFileTypeVideo:
			 
			return video_path_filename.string();

		case Stream::FType::kFileTypeHls:
			 
			return hls_path_filename.string();

		case Stream::FType::kFileTypeRtmp: 
			return "";

		default: 
			return "";
	}
}

std::string RtspStreamHandle::get_error_msg(int nErrorCode)
{
	char szMsg[AV_ERROR_MAX_STRING_SIZE] = { 0 };
	av_make_error_string(szMsg, AV_ERROR_MAX_STRING_SIZE, nErrorCode);
	std::stringstream ss;
	{
		ss << "Code[" << nErrorCode << "]:" << szMsg;
	}
	std::string msg = ss.str();
	ss.clear();
	return ss.str();
}

cv::Mat RtspStreamHandle::avframe_to_mat(const AVFrame* frame)
{
	if (nullptr == frame || 0 == frame->width || 0 == frame->height) {
		return cv::Mat();
	}
	int width = frame->width;
	int height = frame->height;
	cv::Mat image(height, width, CV_8UC3);  //CV_8UC3 RGB格式  CV_8UC2 黑白  1、2、3表示通道数 
	int cvLinesizes[1];
	cvLinesizes[0] = (int)image.step1();

	if (nullptr == m_pSwsCtx) {

		m_pSwsCtx = sws_getContext(width,
			height,
			(AVPixelFormat)frame->format,
			width,
			height,
			AVPixelFormat::AV_PIX_FMT_BGR24,
			SWS_FAST_BILINEAR,
			NULL,
			NULL,
			NULL);
	}
	if (nullptr == m_pSwsCtx) {
		std::cout << "Allocate SwsContext failed.\n";
		return cv::Mat();
	}

	sws_scale(m_pSwsCtx,
		frame->data,
		frame->linesize,
		0,
		height,
		&image.data,
		cvLinesizes);
	return image;
}

CameraConnectingStatus RtspStreamHandle::get_camera_connecting_status()
{
	return cameraConnectingStatus;
}

void RtspStreamHandle::get_stream_info(StreamInfo& streamInfo)
{
	streamInfo = m_infoStream;
	return;
}

/// <summary>
/// 添加一条录像信息到云端
/// </summary>
/// <param name="streamInfo"></param>
/// <param name="startTimestamp"></param>
/// <param name="endTimestamp"></param>
void RtspStreamHandle::addnew_record_file_info(StreamInfo& streamInfo, int64_t& startTimestamp, int64_t& endTimestamp)
{
	//CameraMpegInfo----------------------------------------*****************************************
	Service::CameraMpegInfo cameraMpegInfo;
	cameraMpegInfo.DeviceSerialNo = DEVICE_CONFIG.cfgDevice.device_serial_no;
	cameraMpegInfo.RecordId = endTimestamp;
	cameraMpegInfo.CameraId = streamInfo.nCameraId;
	cameraMpegInfo.IsGroup = false;

	int iCreateTime, iModifyTime, iAccessTime, iFileLen;
	cameraMpegInfo.FileSize = 0;
 
	std::this_thread::sleep_for(std::chrono::milliseconds(2000));

	if (true == File::get_file_info(m_path_filename, iCreateTime, iModifyTime, iAccessTime, iFileLen))
	{
		//TEST
		//LOG(INFO) << "m_path_filename file size (bytes):" << iFileLen << " m_path_filename = " << m_path_filename << endl;
		cameraMpegInfo.FileSize = iFileLen;
	}
	else
	{
		LOG(ERROR) << "RtspStramHandle::get_file_info : no file here:" << m_path_filename;
	}

	cameraMpegInfo.FileFormat = streamInfo.mediaFormate == 0 ? "MP4" : "FLV";
	cameraMpegInfo.StartTimestamp = startTimestamp; //nMillSecond
	cameraMpegInfo.EndTimestamp = endTimestamp;  //nLastSaveVideo 

	/*std::stringstream ss;
	{
		ss << startTimestamp << "." << cameraMpegInfo.FileFormat;
	}*/
	cameraMpegInfo.MpegFileName = m_path_filename; //ss.str();

	//-----------------------------------------------------
	std::string strResponse;
	bool bMpegAddResult = false;
	CameraMpeg cameraMpeg;
	cameraMpeg.camera_mpeg_add2(cameraMpegInfo, strResponse); //test ok
	//printf("camera_mpeg_add response::,%s\n", strResponse.c_str());
}
