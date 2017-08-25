# 查找当前目录下的所有源文件
# 并将名称保存到 DIR_SRCS 变量
aux_source_directory(. DIR_SRCS)
aux_source_directory(../../proto/msg_proto_src MSG_PROTO_SRCS)
list(APPEND DIR_SRCS ${MSG_PROTO_SRCS})

# 设置头文件
include_directories(../../engine/inc)
include_directories(../../3rd/protobuf/include)
include_directories(../../proto)

# 设置输出目录
set(EXECUTABLE_OUTPUT_PATH ${BIN_OUTPUT_PATH})
# 设置链接的库
set(LINK_LIBS ${BIN_OUTPUT_PATH}/libBaseCommon.so ${BIN_OUTPUT_PATH}/libBaseNetwork.so ${PROJECT_SOURCE_DIR}/3rd/protobuf/lib/linux/libprotobuf.a rt dl)
# 设置链接选项
set(CMAKE_EXE_LINKER_FLAGS "-Wl,-rpath=./ ")
# 生成gate
add_executable(sample_client ${DIR_SRCS})

# 链接库
target_link_libraries(sample_client ${LINK_LIBS})