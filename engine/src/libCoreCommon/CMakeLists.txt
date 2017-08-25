# 查找当前目录下的所有源文件
# 并将名称保存到 DIR_SRCS 变量
aux_source_directory(. DIR_SRCS)
list(APPEND DIR_SRCS ./context_linux.S)
aux_source_directory(./proto_src PROTO_SRCS)
list(APPEND DIR_SRCS ${PROTO_SRCS})

# 设置头文件
include_directories(../../../engine/inc/libCoreCommon)
include_directories(../../../engine/inc)
include_directories(../../../3rd/tinyxml2/include)
include_directories(../../../3rd/protobuf/include)

# 设置输出目录
SET(LIBRARY_OUTPUT_PATH ${BIN_OUTPUT_PATH})
#设置汇编
set(CMAKE_ASM_FLAGS "${CFLAGS} -x assembler-with-cpp")
enable_language(C ASM)

set(LINK_LIBS ${PROJECT_SOURCE_DIR}/3rd/protobuf/lib/linux/libprotobuf.a ${BIN_OUTPUT_PATH}/libBaseCommon.so ${BIN_OUTPUT_PATH}/libBaseNetwork.so)

# 生成libCoreCommon.so
add_library(CoreCommon SHARED ${DIR_SRCS})

target_link_libraries(CoreCommon ${LINK_LIBS})

add_custom_command(
	TARGET CoreCommon
    PRE_BUILD
    COMMAND rm -rf ${LIBRARY_OUTPUT_PATH}/libtinyxml2.so
	COMMAND rm -rf ${LIBRARY_OUTPUT_PATH}/libtinyxml2.so.5
)

add_custom_command(
	TARGET CoreCommon
    POST_BUILD
    COMMAND cp -f ${PROJECT_SOURCE_DIR}/3rd/tinyxml2/lib/linux/libtinyxml2.so ${LIBRARY_OUTPUT_PATH}
	COMMAND ln -s ${LIBRARY_OUTPUT_PATH}/libtinyxml2.so ${LIBRARY_OUTPUT_PATH}/libtinyxml2.so.5
)