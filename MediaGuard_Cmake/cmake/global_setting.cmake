#####################################################################
#
# @brief	global setting cmake 
#
#####################################################################

# environment setting file
set(PLAT_TYPE)
set(FILE_CMAKE_ENV)
IF(MSVC)
    set(FILE_CMAKE_ENV msvc_setting.cmake)
    IF(CMAKE_CL_64)
        set(PLAT_TYPE win64)
        set(FILE_CMAKE_ENV msvc_setting.cmake)
    ELSE(CMAKE_CL_64)
        set(PLAT_TYPE win32)
    ENDIF(CMAKE_CL_64)
elseif(APPLE)
    set(FILE_CMAKE_ENV mac_setting.cmake)
elseif(UNIX)
    #set(FILE_CMAKE_ENV linux_environment_setting.cmake)
    set(FILE_CMAKE_ENV linux_setting.cmake)
endif()

message(STATUS "CMAKE_SOURCE_DIR============= ${CMAKE_SOURCE_DIR}")
# Third-party library header files and library file variables
set(MediaGuard_PATH ${CMAKE_SOURCE_DIR}/MediaGuard)
set(MediaGuard_SRC_PATH ${MediaGuard_PATH}/src)
set(MediaGuard_SRC_INCLUDE_PATH ${MediaGuard_SRC_PATH}/include)
set(MediaGuard_SRC_INCLUDE_BASE_PATH ${MediaGuard_SRC_INCLUDE_PATH}/Basic)
set(MediaGuard_SRC_INCLUDE_COMMON_PATH ${MediaGuard_SRC_INCLUDE_PATH}/Common)
set(MediaGuard_SRC_INCLUDE_CONFIG_PATH ${MediaGuard_SRC_INCLUDE_PATH}/Config)
set(MediaGuard_SRC_INCLUDE_EASYLOGGING_PATH ${MediaGuard_SRC_INCLUDE_PATH}/easylogging)
set(MediaGuard_SRC_INCLUDE_ERRORINFO_PATH ${MediaGuard_SRC_INCLUDE_PATH}/ErrorInfo)
set(MediaGuard_SRC_INCLUDE_HTTP_PATH ${MediaGuard_SRC_INCLUDE_PATH}/Http)
set(MediaGuard_SRC_INCLUDE_HTTPLIB_PATH ${CMAKE_SOURCE_DIR}/include/httplib)
set(MediaGuard_SRC_INCLUDE_RAPIDJSON_PATH ${CMAKE_SOURCE_DIR}/include/rapidjson)


# openss1.1.1q   
IF(MSVC)
    set(OPENSSL_INCLUDE_PATH ${CMAKE_SOURCE_DIR}/3rdlib/windows/x64/openssl1.1.1b/include)
    set(LIB_OPENSSL_PATH ${CMAKE_SOURCE_DIR}/3rdlib/windows/x64/openssl1.1.1b/lib)
    link_directories(${LIB_OPENSSL_PATH})
    
    set(OPENSSL_LIB "libcrypto.lib"      
        "libssl.lib" 
    )

elseif(APPLE)

elseif(UNIX)
    set(OPENSSL_INCLUDE_PATH ${CMAKE_SOURCE_DIR}/3rdlib/linux/x64/openssl-1.1.1q/include) 
    set(LIB_OPENSSL_PATH ${CMAKE_SOURCE_DIR}/3rdlib/linux/x64/openssl-1.1.1q)
    link_directories(${LIB_OPENSSL_PATH})
 
    # 將 OpenSSL 的庫文件添加到目標的連接器中 
    set(OPENSSL_LIB PRIVATE OpenSSL::Crypto OpenSSL::SSL)
      
    function(target_link_opencv_libraries arg1)
        foreach(LIB IN LISTS OPENSSL_LIB)
            target_link_libraries(${arg1}  
                ${LIB}
            )
        endforeach() 
    endfunction()
     
endif()


# opencv opencv-4.0.0  
IF(MSVC)
set(OPENCV_INCLUDE_PATH ${MediaGuard_SRC_PATH}/lib/opencv-4.0.0/include)
set(LIB_OPENCV_PATH "${CMAKE_SOURCE_DIR}/Library/opencv-4.0.0/win64")
link_directories(${LIB_OPENCV_PATH}/lib)

set(OPENCV_INCLUDE_PATH ${MediaGuard_SRC_PATH}/lib/opencv-4.0.0/include)
set(LIB_OPENCV_PATH ${MediaGuard_SRC_PATH})
link_directories(${LIB_OPENCV_PATH}/lib)
set(OPCV_LIB "opencv_calib3d400.lib"      
    "opencv_ml400.lib"
    "opencv_calib3d400d.lib"     
    "opencv_ml400d.lib"
    "opencv_core400.lib"         
    "opencv_objdetect400.lib"
    "opencv_core400d.lib"        
    "opencv_objdetect400d.lib"
    "opencv_dnn400.lib"          
    "opencv_photo400.lib"
    "opencv_dnn400d.lib"         
    "opencv_photo400d.lib"
    "opencv_features2d400.lib"   
    "opencv_stitching400.lib"
    "opencv_features2d400d.lib"  
    "opencv_stitching400d.lib"
    "opencv_flann400.lib"        
    "opencv_video400.lib"
    "opencv_flann400d.lib"       
    "opencv_video400d.lib"
    "opencv_gapi400.lib"         
    "opencv_videoio400.lib"
    "opencv_gapi400d.lib"        
    "opencv_videoio400d.lib"
    "opencv_highgui400.lib"      
    "opencv_highgui400d.lib"     
    "opencv_imgcodecs400.lib"    
    "opencv_imgcodecs400d.lib"   
    "opencv_imgproc400.lib"      
    "opencv_imgproc400d.lib"
)
elseif(APPLE)

elseif(UNIX)
    set(OPENCV_INCLUDE_PATH ${CMAKE_SOURCE_DIR}/3rdlib/linux/x64/opencv4.9/include)
    set(LIB_OPENCV_PATH ${CMAKE_SOURCE_DIR}/3rdlib/linux/x64/opencv4.9/)
    link_directories(${LIB_OPENCV_PATH}/lib)
    set(OPCV_LIB libopencv_calib3d.so           
    libopencv_flann.so            
    libopencv_imgproc.so          
    libopencv_stitching.so
    libopencv_core.so              
    libopencv_gapi.so             
    libopencv_ml.so 
    libopencv_videoio.so
    libopencv_dnn.so   
    libopencv_highgui.so          
    libopencv_objdetect.so  
    libopencv_video.so
    libopencv_features2d.so        
    libopencv_imgcodecs.so        
    libopencv_photo.so     
    )
endif()

#提供链接opencv方法
function(target_link_opencv_libraries arg1)
    foreach(LIB IN LISTS OPCV_LIB)
        target_link_libraries(${arg1}  
            debug ${LIB}
        )
    endforeach()
    foreach(LIB IN LISTS OPCV_LIB)
        target_link_libraries(${arg1}  
            optimized ${LIB}
        )
    endforeach()
endfunction()

# curl
if(MSVC)
    set(BUILD_TYPE true)
    if(BUILD_TYPE)
        # set(CURL_INCLUDE_PATH ${MediaGuard_SRC_PATH}/lib/opencv-4.0.0/include)
        set(LIB_CURL_PATH ${CMAKE_SOURCE_DIR}/3rdlib/windows/x64/curl)
        include_directories(${LIB_CURL_PATH}/include)
        link_directories(${LIB_CURL_PATH}/lib)
        set(CURL_LIB libcurl.lib libcrypto.lib)

    else()
    endif()

elseif(APPLE)
elseif(UNIX)
    set(LIB_CURL_PATH ${CMAKE_SOURCE_DIR}/3rdlib/linux/x64/curl)
    include_directories(${LIB_CURL_PATH}/include)
    link_directories(${LIB_CURL_PATH}/lib)

    set(CURL_LIB libcurl.so)
    

    function(target_link_curl_libraries arg1)
    
        foreach(LIB IN LISTS CURL_LIB)
            target_link_libraries(${arg1}  
                debug ${LIB}
            )
        endforeach()
        foreach(LIB IN LISTS CURL_LIB)
            target_link_libraries(${arg1}  
                optimized ${LIB}
            )
        endforeach()
    endfunction()

else()
endif()
 


# 3rdlib
set(THIRD_LIB_PATH ${CMAKE_SOURCE_DIR}/3rdlib)
IF(MSVC)
    set(THIRD_LIB_WINDOWS_PATH ${THIRD_LIB_PATH}/windows)
elseif(APPLE)
elseif(UNIX)
endif()

IF(MSVC)
    # boost
    
elseif(APPLE)

elseif(UNIX)

endif()


#######################################################################################

# ffmpeg
if(MSVC) 

    set(FFMPEG_INCLUDE_PATH "${CMAKE_SOURCE_DIR}/3rdlib/windows/x64/ffmpeg/include")
    include_directories(${FFMPEG_INCLUDE_PATH})

    set(LIB_FFMPEG_PATH "${CMAKE_SOURCE_DIR}/3rdlib/windows/x64/ffmpeg/win64")
    link_directories(${LIB_FFMPEG_PATH}/lib)

elseif(APPLE)
elseif(UNIX)

    set(LIB_FFMPEG_PATH "${CMAKE_SOURCE_DIR}/3rdlib/linux/x64/ffmpeg/")

    include_directories(${LIB_FFMPEG_PATH}/include)
    link_directories(${LIB_FFMPEG_PATH}/lib)

    set(FFMPEG_LIB libavcodec.so           
    libavdevice.so            
    libavfilter.so          
    libavformat.so
    libavutil.so              
    libpostproc.so             
    libswresample.so 
    libswscale.so    
    )

    function(target_link_ffmpeg_libraries arg1)
        foreach(LIB IN LISTS FFMPEG_LIB)
            target_link_libraries(${arg1}  
                debug ${LIB}
            )
        endforeach()
        foreach(LIB IN LISTS FFMPEG_LIB)
            target_link_libraries(${arg1}  
                optimized ${LIB}
            )
        endforeach()
    endfunction()

else()
endif()


message("==========11111======================OPENCV_INCLUDE_PATH: ${OPENCV_INCLUDE_PATH}")
 

SET(SDK_INCLUDE_PATH 
    # ${MediaGuard_SRC_PATH}

    ${OPENSSL_INCLUDE_PATH} 
    ${MediaGuard_SRC_INCLUDE_PATH} 
    ${MediaGuard_SRC_INCLUDE_BASE_PATH} 
    ${MediaGuard_SRC_INCLUDE_COMMON_PATH} 
    ${MediaGuard_SRC_INCLUDE_CONFIG_PATH} 
    ${MediaGuard_SRC_INCLUDE_EASYLOGGING_PATH} 
    ${MediaGuard_SRC_INCLUDE_EASYLOGGING_PATH} 
    ${MediaGuard_SRC_INCLUDE_ERRORINFO_PATH}
    ${MediaGuard_SRC_INCLUDE_HTTP_PATH}
    ${MediaGuard_SRC_INCLUDE_RAPIDJSON_PATH}
    
    ${FFMPEG_INCLUDE_PATH}
    ${OPENCV_INCLUDE_PATH}
    ${INCLUDE_BOOST_PATH}
    ${MediaGuard_SRC_INCLUDE_HTTPLIB_PATH}
)

# library path
# global include path
if(MSVC)
    
elseif(APPLE)
elseif(UNIX)
else()
endif()


# cache the path that store the project
message("Project store path: ${PROJECT_ROOT_DIR}")
set(PROJECT_ROOT_DIR  ${PROJECT_SOURCE_DIR})

# create the include directory
set(INCLUDE_PATH "${PROJECT_ROOT_DIR}/include")
create_directory(${INCLUDE_PATH})
