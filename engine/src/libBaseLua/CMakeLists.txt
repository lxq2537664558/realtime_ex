# 查找当前目录下的所有源文件
# 并将名称保存到 DIR_SRCS 变量
aux_source_directory(. DIR_SRCS)
# 设置头文件
include_directories(../../../engine/inc/libBaseLua)
include_directories(../../../engine/inc)
include_directories(../../../3rd/lua/include)

# 设置输出目录
SET(LIBRARY_OUTPUT_PATH ../../../bin/linux)
# 生成libCoreCommon.so
add_library(BaseLua SHARED ${DIR_SRCS})