INCLUDE_DIR := \
-I../../../engine/inc/libBaseCommon

include ../../../build.mk

BIN_PROJECT_NAME := $(BIN_ROOT_DIR)/libBaseCommon.so

.PHONY: all
all: c_obj cxx_obj
	g++ $(COMPILE_OPTION) -shared -o $(BIN_PROJECT_NAME) ./*.o
	
.PHONY: c_obj
c_obj: $(C_OBJS)

.PHONY: cxx_obj
cxx_obj: $(CXX_OBJS)

.PHONY: clean
clean:
	-rm -f *.o
	-rm -f $(BIN_PROJECT_NAME)
