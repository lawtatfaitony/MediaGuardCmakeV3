#####################################################################
# @brief	utils cmake 
#
#####################################################################


#retrieve file by directory
function(retrieve_files out_files)
	set(file_list)
	foreach(dir ${ARGN})
		# retrieve files for dir into files
		file(GLOB_RECURSE source_files RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
			"${dir}/*.cmake"
            "${dir}/*.h"
            "${dir}/*.hpp"
            "${dir}/*.c"
            "${dir}/*.cpp"
            "${dir}/*.cc"
            "${dir}/*.ui"
            "${dir}/*.h.in"
            "${dir}/*.rc"
		)
		foreach(filename ${source_files})
			list(APPEND file_list "${CMAKE_CURRENT_SOURCE_DIR}/${filename}")
			get_filename_component(file_dir "${filename}" DIRECTORY )
			# setup group
			source_group(${file_dir} FILES "${filename}")
		endforeach()
	endforeach()
	set(${out_files} ${file_list} PARENT_SCOPE)
endfunction()


# list files for directory with specified pattern
function(list_files dir out_files)
	set(file_list)
	foreach(pattern ${ARGN})
		file(GLOB retrieve_files "${dir}/${pattern}")
		list(APPEND file_list ${retrieve_files})
	endforeach()
	set(${out_files} ${file_list} PARENT_SCOPE)
endfunction()


# list files for directory with specified pattern
function(list_files_retrieve dir out_files)
	set(file_list)
	foreach(pattern ${ARGN})
		file(GLOB_RECURSE retrieve_files RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "${dir}/${pattern}")
		list(APPEND file_list ${retrieve_files})
	endforeach()
	set(${out_files} ${file_list} PARENT_SCOPE)
endfunction()


# create directory
function(create_directory)
	foreach(dir ${ARGN})
		if(NOT EXISTS ${dir})
			file(MAKE_DIRECTORY ${dir})
			message(STATUS "Create Directory: ${dir}")
		endif()
	endforeach()
endfunction()


# replace string
function(replace_splash source out_string)
	if(MSVC)
		string(REPLACE "/" "\\" convert_string ${source})
	else()
		string(REPLACE "\\" "/" convert_string ${source})
	endif()
    set(${out_string} ${convert_string} PARENT_SCOPE)
endfunction()


# copy file with common
function(copy_file dest_dir)
    set(file_list)
	foreach(filename ${ARGN}) 
		replace_splash(${filename} filename_convert)
		list(APPEND file_list ${filename_convert})
        #list(APPEND file_list "${CMAKE_CURRENT_SOURCE_DIR}/${filename}")
	endforeach()
    # replash splash
	replace_splash(${dest_dir} dest_dir_convert)
	# copy files
	file(COPY "${file_list}" DESTINATION "${dest_dir_convert}")
endfunction()


# copy file after executed with target
function(copy_by_post source destination build_project)
	replace_splash(${source} source_convert)
	replace_splash(${destination} destination_convert)
	if(MSVC)
		# add custom command with target
		add_custom_command(TARGET ${build_project}
            POST_BUILD
            COMMAND copy /Y "\"${source_convert}\"" "\"${destination_convert}\""
        )
	else()
        add_custom_command(TARGET ${build_project}
            POST_BUILD
            COMMAND cp -f "\"${source_convert}\"" "\"${destination_convert}\""
        )
	endif()
endfunction()


# modify project version
function(modify_project_version)
    string(TIMESTAMP PROJECT_BUILD_YEAR "%Y")
    string(TIMESTAMP PROJECT_BUILD_DATE "%Y%m%d")
    string(TIMESTAMP PROJECT_BUILD_TIME "%H%M%S")
    set(PROJECT_REVISION 0)
    # get svn/git commit reversion
    message("CMAKE_SOURCE_DIR: ${CMAKE_SOURCE_DIR}")
    if(EXISTS "${CMAKE_SOURCE_DIR}/.git/")
        find_package(Git)
        if(GIT_FOUND)
            execute_process(COMMAND ${GIT_EXECUTABLE} describe --tags RESULT_VARIABLE res_code OUTPUT_VARIABLE GIT_COMMIT_ID)
            if(${res_code} EQUAL 0)
                message("-- Get git revision success")
                # -g: tag of git
                string(FIND  ${GIT_COMMIT_ID} "-g" pos)
                if(${pos} GREATER 0)
                    string(SUBSTRING ${GIT_COMMIT_ID} ${pos} -1 COMMIT_ID)
                    string(SUBSTRING ${COMMIT_ID} 2 -1 ${PROJECT_REVISION})
                    message("-- Git commit id: ${PROJECT_REVISION}")
                endif()
            else(${res_code} EQUAL 0)
                message( WARNING "-- Git failed (not a repo, or no tags). Build will not contain git revision info." )
            endif(${res_code} EQUAL 0)
        else(GIT_FOUND)
            message("-- Git not found!)")
        endif(GIT_FOUND)
    else(EXISTS "${CMAKE_SOURCE_DIR}/.git/")
        if(EXISTS "${CMAKE_SOURCE_DIR}/.svn/")
            FIND_PACKAGE(Subversion)
            if(SUBVERSION_FOUND)
                Subversion_WC_INFO(${CMAKE_CURRENT_SOURCE_DIR} Project)
                SET(PROJECT_REVISION ${Project_WC_REVISION})
                message("-- Svn revision:${PROJECT_REVISION}")
            else(SUBVERSION_FOUND)
                message("-- Can't find packet Subversion")
            endif(SUBVERSION_FOUND)
        else()
            message(ERROR "-- Svn directory not exists")
        endif(EXISTS "${CMAKE_SOURCE_DIR}/.svn/")
    endif(EXISTS "${CMAKE_SOURCE_DIR}/.git/")

    # generate the version file
    set(VERSION_FILE ${CMAKE_CURRENT_SOURCE_DIR}/Version/Version.h)
    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/Version/Version.h.in"
		"${VERSION_FILE}"
		@ONLY)
endfunction()

