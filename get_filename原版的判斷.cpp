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
	std::string strFilename;

	switch (ftype)
	{
	case Stream::FType::kFileTypePicture:
		strFilename = kPictureDir + "/" + strTimestamp + kPictureFormat;
		return strFilename;

	case Stream::FType::kFileTypeVideo:
		strFilename = kVideoDir + "/" + m_strToday + "/" + strTimestamp + (m_infoStream.mediaFormate == FLV ? kVidoeFormatFlv : kVidoeFormat);
		return strFilename;

	case Stream::FType::kFileTypeHls:
		strFilename = kHlsDir + "/" + to_string(m_infoStream.nCameraId) + "/index.m3u8";
		return strFilename;

	case Stream::FType::kFileTypeRtmp:
		strFilename = "";
		return strFilename;
	default:
		strFilename = kPictureDir + "/" + strTimestamp + kVidoeFormat;
		return strFilename;
	}
}