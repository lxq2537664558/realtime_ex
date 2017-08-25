# 查找当前目录下的所有源文件
# 并将名称保存到 DIR_SRCS 变量
aux_source_directory(. DIR_SRCS)
aux_source_directory(../../proto/msg_proto_src MSG_PROTO_SRCS)
list(APPEND DIR_SRCS ${MSG_PROTO_SRCS})
aux_source_directory(../../proto/db_proto_src DB_PROTO_SRCS)
list(APPEND DIR_SRCS ${DB_PROTO_SRCS})

# 设置头文件
include_directories(../../engine/inc)
include_directories(../../3rd/tinyxml2/include)
include_directories(../../3rd/protobuf/include)
include_directories(../../proto)

# 设置输出目录
SET(LIBRARY_OUTPUT_PATH ${BIN_OUTPUT_PATH})

set(LINK_LIBS ${PROJECT_SOURCE_DIR}/3rd/protobuf/lib/linux/libprotobuf.a ${BIN_OUTPUT_PATH}/libBaseCommon.so ${BIN_OUTPUT_PATH}/libBaseNetwork.so ${BIN_OUTPUT_PATH}/libCoreCommon.so)

# 生成gate.so
add_library(sample_gas SHARED ${DIR_SRCS})

target_link_libraries(sample_gas ${LINK_LIBS})

add_custom_command(
	TARGET sample_gas
    POST_BUILD
    COMMAND mv ${LIBRARY_OUTPUT_PATH}/libsample_gas.so ${LIBRARY_OUTPUT_PATH}/sample_gas.so
)