#####################################################################
#
# @brief	CMakeLists of root project
#
#####################################################################

# cmake ../ -G "Visual Studio 17 2022"
# cmake ../ -G "Unix Makefiles"

# version of cmake required at least
# cmake_minimum_required(VERSION 3.14)
cmake_minimum_required(VERSION 3.5.1)

 
# project name
set(PROJECT_NAME MediaGuardApp)
project(${PROJECT_NAME})

 message("-------------------------------------------------------------------------------------------")
 message("CMAKE_CURRENT_SOURCE_DIR : "  ${CMAKE_CURRENT_SOURCE_DIR})
 message("-------------------------------------------------------------------------------------------")

# enable folder property
set_property(GLOBAL PROPERTY USE_FOLDERS ON)


#set project group
set(PROJECT_FOLDER_APP "App")
set(PROJECT_FOLDER_STATIC "Static")
set(PROJECT_FOLDER_Shared "Shared")
set(PROJECT_FOLDER_TEST "Test")


# include cmake file
include(cmake/utils.cmake)
include(cmake/global_setting.cmake)
include(cmake/compilerconfig.cmake)

# set output property
set(OUTPUT_TARGET_DIR Output)
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/${OUTPUT_TARGET_DIR})
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/${OUTPUT_TARGET_DIR})


# add project
add_subdirectory(cmake)
add_subdirectory(Library/HelloWorld)
add_subdirectory(Library/MathLibrary)
add_subdirectory(Library/CustomLib)
add_subdirectory(MediaGuard)


# add test project
add_subdirectory(Test/TestMath)



