# 查找当前目录下的所有源文件
# 并将名称保存到 DIR_SRCS 变量
aux_source_directory(. LIST_SRCS)
aux_source_directory(./proto_src PROTO_SRCS)
list(APPEND LIST_SRCS ${PROTO_SRCS})

# 设置头文件
include_directories(../../engine/inc)
include_directories(../../3rd/tinyxml2/include)
include_directories(../../3rd/lua/include)
include_directories(../../3rd/protobuf/include)

# 设置输出目录
set(EXECUTABLE_OUTPUT_PATH ${BIN_OUTPUT_PATH})
# 设置链接的库
set(LINK_LIBS libBaseCommon.so libBaseNetwork.so libBaseLua.so libCoreCommon.a libCoreServiceKit.a libprotobuf.a libtinyxml2.a rt dl)
# 设置链接选项
set(CMAKE_EXE_LINKER_FLAGS "")
# 设置运行时查找lib的路径
link_directories(${PROJECT_SOURCE_DIR}/3rd/tinyxml2/lib/linux/)
link_directories(${PROJECT_SOURCE_DIR}/3rd/protobuf/lib/linux/)
link_directories(${BIN_OUTPUT_PATH})
# 生成gate
add_executable(gate ${LIST_SRCS})

# 链接库
target_link_libraries(gate ${LINK_LIBS})