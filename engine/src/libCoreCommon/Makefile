INCLUDE_DIR := \
-I../../../engine/inc/libCoreCommon\
-I../../../engine/inc\
-I../../../3rd/tinyxml2/include

include ../../../build.mk

BIN_PROJECT_NAME := $(BIN_ROOT_DIR)/libCoreCommon.a

.PHONY: all
all: c_obj cxx_obj
	ar rcs $(BIN_PROJECT_NAME) ./*.o
	cp ../../../3rd/tinyxml2/lib/linux/libtinyxml2.so $(BIN_ROOT_DIR)/
	
.PHONY: c_obj
c_obj: $(C_OBJS)

.PHONY: cxx_obj
cxx_obj: $(CXX_OBJS)

.PHONY: clean
clean:
	-rm -f *.o
	-rm -f $(BIN_PROJECT_NAME)
	-rm -f $(BIN_ROOT_DIR)/libtinyxml2.so
