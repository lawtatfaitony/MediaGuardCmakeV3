#####################################################################
#
# @brief	CMakeLists of project MediaGuard
#
#####################################################################


#set project name
set(PROJECT_NAME TestMath)
project(${PROJECT_NAME})


message("===================Build ${PROJECT_NAME} begin...")
# include cmake file
include(cmake/common_setting.cmake)
include(cmake/${FILE_CMAKE_ENV})


# print the source files
foreach(file ${SOURCE_FILES})
    message("source file: ${file}")
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
    message("defines:\t ${def}")
endforeach()


# include directory
message("Print include directory")
include_directories(${PROJECT_INCLUDE_DIRECTORY})
get_property(inc_dir 
    DIRECTORY
    ${CMAKE_CURRENT_SOURCE_DIR}
    PROPERTY
    INCLUDE_DIRECTORIES
)
foreach(dir ${inc_dir})
    message("include dir: \t ${dir}")
endforeach()


# set link directory
message("Set link directory")
link_directories($<IF:$<CONFIG:Debug>,${PROJECT_LINK_DIRECTORY_DEBUG},${PROJECT_LINK_DIRECTORY_RELEASE}>)
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


# link library(or link libraries)
message("Set link library")
set_property(TARGET 
   ${PROJECT_NAME}
   PROPERTY
   LINK_LIBRARIES
   $<IF:$<CONFIG:Debug>,${PROJECT_LINK_LIBRARIES_DEBUG},${PROJECT_LINK_LIBRARIES_RELEASE}>
)
get_property(link_lib 
    TARGET
    ${PROJECT_NAME}
    PROPERTY
    LINK_LIBRARIES
)
foreach(lib ${link_lib})
    message("link library: \t ${lib}")
endforeach()


# setup the group
message("--Set up the group")
set_property(TARGET 
    ${PROJECT_NAME}
    PROPERTY FOLDER
    ${PROJECT_FOLDER_TEST}
)
message(STATUS "target group: ${PROJECT_FOLDER_TEST}")

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/src/include/ 
)
# set debugger ervironment
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