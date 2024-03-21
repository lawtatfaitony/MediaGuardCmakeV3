#####################################################################
#
# @brief	CMakeLists of project
# @date  	2021/06/30
# @auth		Wite_Chen
#
#####################################################################

# cmake ../ -G "Visual Studio 17 2022"
# cmake ../ -G "Unix Makefiles"


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#set project name
set(PROJECT_NAME MediaGuard)


project(${PROJECT_NAME})
 
 message("MediaGuard----------------------------------------------------------------------------------------------")
 message("CMAKE_CURRENT_SOURCE_DIR = " ${CMAKE_CURRENT_SOURCE_DIR}) 
 message("MediaGuard----------------------------------------------------------------------------------------------")
 
message("===================Build ${PROJECT_NAME} begin...")
# include cmake file
include(cmake/common_setting.cmake)
include(cmake/${FILE_CMAKE_ENV})

# set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 17)

# print the source files
foreach(file ${SOURCE_FILES})
    # message(${PROJECT_NAME} "source file: ${file}")
endforeach()


# add defines
message("Print compile defines")

add_definitions(${PROJECT_COMPILE_DEFINES})

get_property(compile_define
    DIRECTORY
    ${CMAKE_CURRENT_SOURCE_DIR}
    PROPERTY
    COMPILE_DEFINITIONS
)
foreach(def ${compile_define})
    message(${PROJECT_NAME} " defines:\t ${def}")
endforeach()


# include directory
message("Print include directory")
# e.g.   E:/MEDIAGUARD_CMAKE/MediaGuard_Cmake/MediaGuard_Cmake/include

IF(MSVC)
message("2222222222222222 MSVC")
    include_directories(
        ${PROJECT_INCLUDE_DIRECTORY}
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    )
elseif(APPLE)

elseif(UNIX)
endif()

get_property(inc_dir 
    DIRECTORY
    ${CMAKE_CURRENT_SOURCE_DIR}
    PROPERTY
    INCLUDE_DIRECTORIES
)
foreach(dir ${inc_dir})
    message(${PROJECT_NAME} "include dir: \t ${dir}")
endforeach()


# set link directory
message("Set link directory")

# include local header files
include_directories(${SDK_INCLUDE_PATH})

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/
    ${CMAKE_CURRENT_SOURCE_DIR}/include/Basic/
    ${CMAKE_CURRENT_SOURCE_DIR}/include/Common/
	${CMAKE_CURRENT_SOURCE_DIR}/include/Config/
	${CMAKE_CURRENT_SOURCE_DIR}/include/easylogging/
	${CMAKE_CURRENT_SOURCE_DIR}/include/ErrorInfo/
	${CMAKE_CURRENT_SOURCE_DIR}/include/ffmpeg/include/
	${CMAKE_CURRENT_SOURCE_DIR}/include/libcurl/
	${CMAKE_CURRENT_SOURCE_DIR}/include/rapidjson/
	${CMAKE_CURRENT_SOURCE_DIR}/include/httplib/
)
IF(MSVC)
    message("11111111111111111 MSVC")
    include_directories(
	    ${CMAKE_CURRENT_SOURCE_DIR}/include/opencv-4.0.0/
	    ${CMAKE_CURRENT_SOURCE_DIR}/include/opencv-4.0.0/include
    )
elseif(APPLE)

elseif(UNIX)
endif()


IF(MSVC)
link_directories($<IF:$<CONFIG:Debug>,${PROJECT_LINK_DIRECTORY_DEBUG},${PROJECT_LINK_DIRECTORY_RELEASE}>)
elseif(APPLE)

elseif(UNIX)
endif()

get_property(link_dir 
    DIRECTORY
    ${CMAKE_CURRENT_SOURCE_DIR}
    PROPERTY
    LINK_DIRECTORIES
)

message("Print link directory")

foreach(dir ${link_dir})
    message("link dir: \t ${dir}")
endforeach()
 
# add target
add_executable(${PROJECT_NAME} ${SOURCE_FILES})


# set link libraries
if(MSVC)
    message("Set link library")
    set_property(TARGET 
    ${PROJECT_NAME}
    PROPERTY
    LINK_LIBRARIES
    $<IF:$<CONFIG:Debug>,${PROJECT_LINK_LIBRARIES_DEBUG},${PROJECT_LINK_LIBRARIES_RELEASE}>)
    get_property(link_lib 
        TARGET
        ${PROJECT_NAME}
        PROPERTY
        LINK_LIBRARIES
    )
    foreach(lib ${link_lib})
        message("link library: \t ${lib}")
    endforeach()
endif()


# setup the group
message("--Set up the group")
set_property(TARGET 
    ${PROJECT_NAME}
    PROPERTY FOLDER
    ${PROJECT_FOLDER_APP}
)
message(STATUS "target group: ${PROJECT_FOLDER_APP}")


# target link library
target_link_libraries(${PROJECT_NAME} ${PROJECT_DEPENDANCE})

# link boost
# target_link_boost_libraries(${PROJECT_NAME})

# link opencv
target_link_opencv_libraries(${PROJECT_NAME})
if(UNIX)
# link linux thread lib
target_link_libraries(${PROJECT_NAME} pthread)
# link linux ffmpeg
target_link_ffmpeg_libraries(${PROJECT_NAME})
#link linux curl
target_link_curl_libraries(${PROJECT_NAME})

# boost 的system库
# target_link_libraries(${PROJECT_NAME} boost_filesystem)
# find_package(Boost REQUIRED COMPONENTS system)
# include_directories(${Boost_INCLUDE_DIRS})
# target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES})

endif()

if(MSVC)
    message("Set debugger environment")
	
    # set debuger directory
    set_property(TARGET 
       ${PROJECT_NAME}
       PROPERTY
       ${VS_DEBUGGER_ENVIRONMENT}
       $<IF:$<CONFIG:Debug>,${DEBUGGER_ENVIRONMENT_DEBUG},${DEBUGGER_ENVIRONMENT_RELEASE}>
    )
	
    # get debugger environment
    get_property(debug_env 
        TARGET
        ${PROJECT_NAME}
        PROPERTY
        VS_DEBUGGER_ENVIRONMENT
    )
    message("--Print the debugger directories")
	
    foreach(dir ${debug_env})
        message("debugger dir: \t${dir}")
    endforeach()
	
endif()


message("===================Build ${PROJECT_NAME} end...")

message("------------------------------------------------------------------------")