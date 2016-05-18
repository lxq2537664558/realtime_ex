# cmake最低版本号
cmake_minimum_required (VERSION 2.8)
# 项目名字
project (realtime_ex)
# cmake额外执行的shell脚本
add_custom_command(TARGET ALL
    COMMAND mkdir bin
	COMMAND mkdir bin/linux
)
#设置编译版本类型
set(CMAKE_BUILD_TYPE "Debug")
#设置C++ Debug 版本编译选项
set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -g -rdynamic -Wall -Wno-reorder -O0 -D _DEBUG -finput-charset=gbk -fPIC --std=c++11")
#设置C++ Release 版本编译选项
set(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -g -rdynamic -Wall -Wno-reorder -O2 -finput-charset=gbk -fPIC --std=c++11")

#添加子目录
add_subdirectory(engine/src)