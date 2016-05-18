# CMake 最低版本号要求
cmake_minimum_required (VERSION 2.8)
# 项目信息
project (realtime_ex)
set(CMAKE_BUILD_TYPE "Debug")
set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -g -rdynamic -Wall -Wno-reorder -O0 -finput-charset=gbk -fPIC --std=c++11")
set(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -g -rdynamic -Wall -Wno-reorder -O2 -finput-charset=gbk -fPIC --std=c++11")
# 添加子目录
add_subdirectory(engine/src)