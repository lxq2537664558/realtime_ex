# 查找当前目录下的所有源文件
# 并将名称保存到 DIR_SRCS 变量
aux_source_directory(. DIR_SRCS)
# 设置头文件
include_directories(../../../engine/inc/libCoreServiceKit)
include_directories(../../../engine/inc)
include_directories(../../../3rd/tinyxml2/include)
include_directories(../../../3rd/lua/include)
include_directories(../../../3rd/protobuf/include)

# 设置输出目录
SET(LIBRARY_OUTPUT_PATH ${BIN_OUTPUT_PATH})

# 生成libCoreServiceKit.a
add_library(CoreServiceKit ${DIR_SRCS})
target_link_libraries(CoreServiceKit libCoreCommon.a)