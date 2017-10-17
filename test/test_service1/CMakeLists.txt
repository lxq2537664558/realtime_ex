# 查找当前目录下的所有源文件
# 并将名称保存到 DIR_SRCS 变量
aux_source_directory(. DIR_SRCS)
aux_source_directory(../proto_src PROTO_SRCS)
list(APPEND DIR_SRCS ${PROTO_SRCS})

# 设置头文件
include_directories(../../engine/inc)
include_directories(../../3rd/tinyxml2/include)
include_directories(../../3rd/protobuf/include)

# 设置输出目录
SET(LIBRARY_OUTPUT_PATH ${BIN_OUTPUT_PATH})

set(LINK_LIBS ${PROJECT_SOURCE_DIR}/3rd/protobuf/lib/linux/libprotobuf.a)

# 生成test_service1.so
add_library(test_service1 SHARED ${DIR_SRCS})

target_link_libraries(test_service1 ${LINK_LIBS})

add_custom_command(
	TARGET test_service1
    POST_BUILD
    COMMAND mv ${LIBRARY_OUTPUT_PATH}/libtest_service1.so ${LIBRARY_OUTPUT_PATH}/test_service1.so
)