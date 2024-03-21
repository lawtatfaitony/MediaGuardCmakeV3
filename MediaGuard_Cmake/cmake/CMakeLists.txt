#####################################################################
#
# @brief	CMakeLists of build project 
#
#####################################################################


#set project name
set(PROJECT_NAME cmake_setting)
project(${PROJECT_NAME})
message("===================Build ${PROJECT_NAME} begin...")


# list files of cmake
message("Current source dir: ${CMAKE_CURRENT_SOURCE_DIR}")
message("Retrieve cmake files...")
list_files_retrieve(${CMAKE_CURRENT_SOURCE_DIR} CMAKE_FILES "*.cmake")
foreach(file ${CMAKE_FILES})
    message("cmake file: " ${file})
endforeach()


# set source files
message("Print source files...")
set(SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp)
foreach(file ${SOURCE_FILES})
    message("souce file: " ${file})
endforeach()


# source group
message("Setup source group...")
# it won't work it the source not include with target
source_group(src FILES ${SOURCE_FILES})
source_group(cmake FILES ${CMAKE_FILES})


# add target
message("Add target...")
add_executable(${PROJECT_NAME} ${CMAKE_FILES} ${SOURCE_FILES})


# set up the group
message("--Setup target group")
set_property(TARGET 
    ${PROJECT_NAME}
    PROPERTY FOLDER
    ${PROJECT_FOLDER_APP}
)
message(STATUS "target group: ${PROJECT_FOLDER_APP}")



message("===================Build ${PROJECT_NAME} end...")