# 查找当前目录下的所有源文件
# 并将名称保存到 DIR_SRCS 变量
aux_source_directory(. DIR_SRCS)

include_directories(../../../engine/inc/libBaseCommon)
# 生成libCoreCommon.so
add_library(libCoreCommon SHARED ${DIR_SRCS})