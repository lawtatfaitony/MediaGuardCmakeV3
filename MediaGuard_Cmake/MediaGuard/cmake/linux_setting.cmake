#####################################################################
#
# @brief	linux setting cmake
#
#####################################################################


#------------------------------include directory------------------------------#
# include directory
set(PROJECT_INCLUDE_DIRECTORY
    ${INCLUDE_DIRECTORIES}
    "${INCLUDE_PATH}"

)

#------------------------------link directory------------------------------#
# link directory
set(PROJECT_LINK_DDIRECTORY
    ${LINK_DIRECTORIES}
)

# link directory(debug)
set(PROJECT_LINK_DDIRECTORY_DEBUG
    ${PROJECT_LINK_DDIRECTORY}
)

# link directory(release)
set(PROJECT_LINK_DDIRECTORY_RELEASE
    ${PROJECT_LINK_DDIRECTORY}
)

#------------------------------link library------------------------------#
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

    libssl.lib
    libcrypto.lib
)

# link library(debug)
set(PROJECT_LINK_LIBRARIES_DEBUG
    ${PROJECT_LINK_LIBRARIES}
)

# link library(release)
set(PROJECT_LINK_LIBRARIES_RELEASE
    ${PROJECT_LINK_LIBRARIES}
)


#------------------------------defines------------------------------#
set(PROJECT_COMPILE_DEFINES
#    -DUNICODE
#    -D_UNICODE
     -std=c++17
)


#------------------------------compile options------------------------------#
set(PROJECT_COMPILE_OPTION
    
)
