CXXFLAGS	:= $(INCLUDE_DIR) -g -rdynamic -Wall -Wno-reorder $(COMPILE_OPTION) -finput-charset=gbk -fPIC --std=c++11
CFLAGS		:= $(INCLUDE_DIR) -g -rdynamic -Wall -Wno-reorder $(COMPILE_OPTION) -finput-charset=gbk -fPIC

# 推导出所有的c obj文件
C_OBJS := $(patsubst %.c, %.o, $(wildcard *.c))
# 推导出所有的cpp obj文件
CXX_OBJS := $(patsubst %.cpp, %.o, $(wildcard *.cpp))

define build_obj
	for SubDir in $(SRC_DIRS); do\
		echo "Building $$SubDir ...";\
		make -C $$SubDir;\
	done
endef

# 编译所有的.c文件为对应的obj文件
%.o: %.c
	@gcc ${CFLAGS} -c $< -o $@
	@echo gcc ${CFLAGS} $< -o $@
	
# 编译所有的.cpp文件为对应的obj文件
%.o: %.cpp
	@g++ ${CXXFLAGS} -c $< -o $@
	@echo g++ ${CFLAGS} $< -o $@
