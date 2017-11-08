# 查找当前目录下的所有源文件
# 并将名称保存到 DIR_SRCS 变量
aux_source_directory(. DIR_SRCS)

# 设置头文件
include_directories(../../../../../engine/inc)
include_directories(../../../../../3rd/gtest/include)

# 设置输出目录
set(EXECUTABLE_OUTPUT_PATH ${BIN_OUTPUT_PATH})

link_directories(${PROJECT_SOURCE_DIR}/3rd/gtest/lib/linux/)
# 设置链接的库
set(LINK_LIBS ${BIN_OUTPUT_PATH}/libBaseCommon.so gtest rt dl pthread)
# 设置链接选项
set(CMAKE_EXE_LINKER_FLAGS "-Wl,-rpath=./ ")
# 生成TestTicker
add_executable(TestTicker ${DIR_SRCS})

# 链接库
target_link_libraries(TestTicker ${LINK_LIBS})