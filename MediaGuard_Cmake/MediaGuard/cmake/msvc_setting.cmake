#####################################################################
#
# @brief	msvc setting cmake 
#
#####################################################################
 
set(3RD_LIBRARY_DIR ${CMAKE_SOURCE_DIR}/3rdlib/windows/x64 CACHE INTERNAL "Third party library directory")

set(INCLUDE_PATH ${CMAKE_SOURCE_DIR}/MediaGuard/src/include/)

set(MediaGuard_PATH ${CMAKE_SOURCE_DIR}/MediaGuard)
set(MediaGuard_SRC_PATH ${MediaGuard_PATH}/src)

#------------------------------include directory------------------------------#
# include directory
set(PROJECT_INCLUDE_DIRECTORY 

    ${INCLUDE_PATH}
    #--------------------------------------------------------------------
    ${3RD_LIBRARY_DIR}/ffmpeg/include/
	
	${3RD_LIBRARY_DIR}/opencv-4.0.0/include/

    ${3RD_LIBRARY_DIR}/curl/include/

    ${3RD_LIBRARY_DIR}/openssl-1.1.1q/include

    ${3RD_LIBRARY_DIR}/rapidjson/include/

    #--------------------------------------------------------------------
    ${MediaGuard_SRC_PATH}/include/common/

    ${MediaGuard_SRC_PATH}/include/Basic/

    ${MediaGuard_SRC_PATH}/include/hmac/

    $(MediaGuard_SRC_PATH)include/easylogging/

    $(MediaGuard_SRC_PATH)include/ErrorInfo/

    $(MediaGuard_SRC_PATH)include/Config/

    ${MediaGuard_SRC_PATH}include/httplib/ 
)

include_directories( 
     PROJECT_INCLUDE_DIRECTORY
)

# openssl-1.1.1q
set(OPENSSL_ROOT_DIR  ${3RD_LIBRARY_DIR}/openssl-1.1.1q)
# 出于兼容性考虑，设置两个额外的变量  
set(OPENSSL_SSL_LIBRARY  ${3RD_LIBRARY_DIR}/openssl-1.1.1q/lib)
set(OPENSSL_SSL_LIBRARIES ${OPENSSL_SSL_LIBRARY})

set(OPENSSL_CRYPTO_LIBRARY  ${3RD_LIBRARY_DIR}/openssl-1.1.1q)
set(OPENSSL_CRYPTO_LIBRARIES ${OPENSSL_CRYPTO_LIBRARY})

#------------------------------link directory------------------------------#
# link directory
set(PROJECT_LINK_DIRECTORY

    ${LINK_DIRECTORIES}

    ${3RD_LIBRARY_DIR}/ffmpeg/win64/lib
    ${3RD_LIBRARY_DIR}/opencv-4.0.0/win64/lib 
     
)

# link directory(debug)
set(PROJECT_LINK_DIRECTORY_DEBUG
    ${PROJECT_LINK_DIRECTORY} 
    ${3RD_LIBRARY_DIR}/libcurl/win64/debug
)

# link directory(release)
set(PROJECT_LINK_DIRECTORY_RELEASE
    ${PROJECT_LINK_DIRECTORY}
    ${3RD_LIBRARY_DIR}/libcurl/win64/release 
)

#------------------------------link library files------------------------------#
# link library
set(PROJECT_LINK_LIBRARIES
    ${LINK_LIBRARIES} 
    CustomLib

    avcodec.lib
    avformat.lib
    avutil.lib
    swscale.lib
    avdevice.lib
    avfilter.lib 
    swresample.lib
    postproc.lib
 
    opencv_core400.lib
    opencv_imgcodecs400.lib
    opencv_imgproc400.lib

    libssl
    libcrypto
    ws2_32.lib
)

# link library(debug)
set(PROJECT_LINK_LIBRARIES_DEBUG
    ${PROJECT_LINK_LIBRARIES}
   
    libcurl_debug.lib 
)

# link library(release)
set(PROJECT_LINK_LIBRARIES_RELEASE
    ${PROJECT_LINK_LIBRARIES}
   
    libcurl.lib 
)  

#------------------------------defines------------------------------#
# set(PROJECT_COMPILE_DEFINES
#    -DUNICODE
#    -D_UNICODE  from -std=c++11 to -std=c++17
#    -std=c++17
# )


#------------------------------compile options------------------------------#
set(PROJECT_COMPILE_OPTION
    
)
 
#------------------------------debugger environment------------------------------#
set(DEBUGGER_ENVIRONMENT_DEBUG
    "PATH=$(Path)"

)

set(DEBUGGER_ENVIRONMENT_RELEASE
    "PATH=$(Path)"

)