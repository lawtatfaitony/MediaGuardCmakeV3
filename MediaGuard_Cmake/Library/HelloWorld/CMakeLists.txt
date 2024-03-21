#####################################################################
#
# @brief	CMakeLists of build project
# @date  	2021/06/30
# @auth		Wite_Chen
#
#####################################################################


#set project name
set(PROJECT_NAME HelloWorld)
project(${PROJECT_NAME})
message("===================Build ${PROJECT_NAME} begin...")


# retriece files 
message("Retrieve source files:")
retrieve_files(SOURCE_FILES
	"src"
	"cmake"
)
foreach(file ${SOURCE_FILES})
    message("source file: ${file}")
endforeach()


# add target(library)
add_library(${PROJECT_NAME} ${SOURCE_FILES})


# setup the group
message("Setup the group")
set_property(TARGET 
    ${PROJECT_NAME}
    PROPERTY FOLDER
    ${PROJECT_FOLDER_STATIC}
)
message(STATUS "target group: ${PROJECT_FOLDER_APP}")

# copy header files
set(INCLUDE_HEADER_DIR ${INCLUDE_PATH}/${PROJECT_NAME})
create_directory("${INCLUDE_HEADER_DIR}")
message("Include header dir: ${INCLUDE_HEADER_DIR}")
copy_file(${INCLUDE_HEADER_DIR} "${CMAKE_CURRENT_SOURCE_DIR}/src/HelloWorld.h")


message("===================Build ${PROJECT_NAME} end...")
message("------------------------------------------------------------------------")


