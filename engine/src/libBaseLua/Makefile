INCLUDE_DIR := \
-I../../../engine/inc/libBaseLua\
-I../../../engine/inc\
-I../../../3rd/lua/include

include ../../../build.mk

BIN_PROJECT_NAME := $(BIN_ROOT_DIR)/libBaseLua.so

.PHONY: all
all: c_obj cxx_obj
	g++ $(COMPILE_OPTION) -shared -fPIC -o $(BIN_PROJECT_NAME) ./*.o
	cp ../../../3rd/lua/lib/linux/liblua.so $(BIN_ROOT_DIR)/
	
.PHONY: c_obj
c_obj: $(C_OBJS)

.PHONY: cxx_obj
cxx_obj: $(CXX_OBJS)

.PHONY: clean
clean:
	-rm -f $(BIN_PROJECT_NAME)
