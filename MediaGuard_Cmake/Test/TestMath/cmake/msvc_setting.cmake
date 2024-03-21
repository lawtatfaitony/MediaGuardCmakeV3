#####################################################################
#
# @brief	msvc setting cmake
# @date  	2021/06/30
# @auth		Wite_Chen
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
set(PROJECT_LINK_DIRECTORY
    ${LINK_DIRECTORIES}
)

# link directory(debug)
set(PROJECT_LINK_DIRECTORY_DEBUG
    ${PROJECT_LINK_DIRECTORY}
)

# link directory(release)
set(PROJECT_LINK_DIRECTORY_RELEASE
    ${PROJECT_LINK_DIRECTORY}
)

#------------------------------link library------------------------------#
# link library
set(PROJECT_LINK_LIBRARIES
    ${LINK_LIBRARIES}
)

# link library(debug)
set(PROJECT_LINK_LIBRARIES_DEBUG
    ${PROJECT_LINK_LIBRARIES}
    HelloWorld
	MathLibrary
)

# link library(release)
set(PROJECT_LINK_LIBRARIES_RELEASE
    ${PROJECT_LINK_LIBRARIES}
    HelloWorld
	MathLibrary
)


#------------------------------defines------------------------------#
# set(PROJECT_COMPILE_DEFINES
#    -DUNICODE
#    -D_UNICODE
#    -std=c++11
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