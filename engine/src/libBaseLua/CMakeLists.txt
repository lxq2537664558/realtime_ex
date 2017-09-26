# 查找当前目录下的所有源文件
# 并将名称保存到 DIR_SRCS 变量
aux_source_directory(. DIR_SRCS)
# 设置头文件
include_directories(../../../engine/inc/libBaseLua)
include_directories(../../../engine/inc)
include_directories(../../../3rd/lua/include)

# 设置输出目录
SET(LIBRARY_OUTPUT_PATH ${BIN_OUTPUT_PATH})

link_directories(${PROJECT_SOURCE_DIR}/3rd/lua/lib/linux/)
# 生成libBaseLua.so
add_library(BaseLua SHARED ${DIR_SRCS})
target_link_libraries(BaseLua lua)

add_custom_command(
	TARGET BaseLua
    POST_BUILD
    COMMAND cp -f ${PROJECT_SOURCE_DIR}/3rd/lua/lib/linux/liblua.so ${LIBRARY_OUTPUT_PATH}
)