# 查找当前目录下的所有源文件
# 并将名称保存到 DIR_SRCS 变量
aux_source_directory(. DIR_SRCS)

# 设置头文件
include_directories(../../engine/inc)
include_directories(../../3rd/tinyxml2/include)
include_directories(../../3rd/protobuf/include)

# 设置输出目录
SET(LIBRARY_OUTPUT_PATH ${BIN_OUTPUT_PATH})

set(LINK_LIBS ${PROJECT_SOURCE_DIR}/3rd/protobuf/lib/linux/libprotobuf.a ${BIN_OUTPUT_PATH}/libBaseCommon.so ${BIN_OUTPUT_PATH}/libBaseNetwork.so ${BIN_OUTPUT_PATH}/libCoreCommon.so)

# master.so
add_library(master SHARED ${DIR_SRCS})

target_link_libraries(master ${LINK_LIBS})

add_custom_command(
	TARGET master
    POST_BUILD
    COMMAND mv ${LIBRARY_OUTPUT_PATH}/libmaster.so ${LIBRARY_OUTPUT_PATH}/master.so 
)