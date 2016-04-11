INCLUDE_DIR := \
-I../../../engine/inc/libBaseDatabase\
-I../../../engine/inc\
-I../../../3rd/mysql/include

include ../../../build.mk

BIN_PROJECT_NAME := $(BIN_ROOT_DIR)/libBaseDatabase.so

.PHONY: all
all: c_obj cxx_obj
	g++ $(COMPILE_OPTION) -shared -o $(BIN_PROJECT_NAME) ./*.o
	cp ../../../3rd/mysql/lib/linux/libmysqlclient_r.so $(BIN_ROOT_DIR)/
	
.PHONY: c_obj
c_obj: $(C_OBJS)

.PHONY: cxx_obj
cxx_obj: $(CXX_OBJS)

.PHONY: clean
clean:
	-rm -f *.o
	-rm -f $(BIN_PROJECT_NAME)
	-rm -f $(BIN_ROOT_DIR)/libmysqlclient_r.so
