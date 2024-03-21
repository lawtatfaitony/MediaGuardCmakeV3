#####################################################################
# @brief	utils cmake 
# 指定編譯版本 ref : obs
#####################################################################


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -Wall -O0 -g -D_MWAITXINTRIN_H_INCLUDED")# -fsanitize=address -fno-omit-frame-pointer")


#------------------------------defines method II for ref------------------------------#
# set(PROJECT_COMPILE_DEFINES
#    -DUNICODE
#    -D_UNICODE  from -std=c++11 to -std=c++17
#     -std=c++17
# )