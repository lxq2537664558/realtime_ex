# 查找当前目录下的所有源文件
# 并将名称保存到 DIR_SRCS 变量

aux_source_directory(. DIR_SRCS)
aux_source_directory(./proto_src PROTO_SRCS)
list(APPEND DIR_SRCS ${PROTO_SRCS})

# 设置头文件
include_directories(../../../3rd/protobuf/include)
include_directories(../../../3rd/mysql/include)
include_directories(../../../engine/inc/libBaseDatabase)
include_directories(../../../engine/inc)

# 设置输出目录
SET(LIBRARY_OUTPUT_PATH ${BIN_OUTPUT_PATH})

link_directories(${PROJECT_SOURCE_DIR}/3rd/protobuf/lib/linux/ ${PROJECT_SOURCE_DIR}/3rd/mysql/lib/linux/)

set(LINK_LIBS protobuf mysqlclient_r ${BIN_OUTPUT_PATH}/libBaseCommon.so)

# libBaseDatabase.so
add_library(BaseDatabase SHARED ${DIR_SRCS})

target_link_libraries(BaseDatabase ${LINK_LIBS})

add_custom_command(
	TARGET BaseDatabase
    PRE_BUILD
	COMMAND cp -f ${PROJECT_SOURCE_DIR}/3rd/mysql/lib/linux/libmysqlclient_r.so ${BIN_OUTPUT_PATH}
)