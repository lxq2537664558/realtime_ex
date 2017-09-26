# 查找当前目录下的所有源文件
# 并将名称保存到 DIR_SRCS 变量
aux_source_directory(. DIR_SRCS)
aux_source_directory(../proto/server_proto_src SERVER_PROTO_SRCS)
list(APPEND DIR_SRCS ${SERVER_PROTO_SRCS})
aux_source_directory(../proto/db_proto_src DB_PROTO_SRCS)
list(APPEND DIR_SRCS ${DB_PROTO_SRCS})

# 设置头文件
include_directories(../../engine/inc)
include_directories(../../3rd/tinyxml2/include)
include_directories(../../3rd/protobuf/include)
include_directories(../proto)

# 设置输出目录
SET(LIBRARY_OUTPUT_PATH ${BIN_OUTPUT_PATH})

set(LINK_LIBS ${PROJECT_SOURCE_DIR}/3rd/protobuf/lib/linux/libprotobuf.a ${BIN_OUTPUT_PATH}/libBaseCommon.so ${BIN_OUTPUT_PATH}/libBaseNetwork.so ${BIN_OUTPUT_PATH}/libCoreCommon.so)

# 生成dispatch_service.so
add_library(dispatch_service SHARED ${DIR_SRCS})

target_link_libraries(dispatch_service ${LINK_LIBS})

add_custom_command(
	TARGET dispatch_service
    POST_BUILD
    COMMAND mv ${LIBRARY_OUTPUT_PATH}/libdispatch_service.so ${LIBRARY_OUTPUT_PATH}/dispatch_service.so
)