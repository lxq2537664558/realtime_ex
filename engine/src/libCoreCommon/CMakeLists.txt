# 查找当前目录下的所有源文件
# 并将名称保存到 DIR_SRCS 变量
aux_source_directory(. DIR_SRCS)
# 设置头文件
include_directories(../../../engine/inc/libCoreCommon)
include_directories(../../../engine/inc)
include_directories(../../../3rd/tinyxml2/include)

# 设置输出目录
SET(LIBRARY_OUTPUT_PATH ../../../bin/linux)

# 生成libCoreCommon.so
add_library(CoreCommon ${DIR_SRCS})

# cmake额外执行的shell脚本
add_custom_command(TARGET ALL
    COMMAND cp ../../../3rd/tinyxml2/lib/linux/libtinyxml2.so ../../../bin/linux
)