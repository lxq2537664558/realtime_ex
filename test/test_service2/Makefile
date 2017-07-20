INCLUDE_DIR := \
-I../../engine/inc\
-I../../3rd/tinyxml2/include

include ../../build.mk

.PHONY: all
all: c_obj cxx_obj
	cd $(BIN_ROOT_DIR);g++ $(COMPILE_OPTION) -g -rdynamic -o TestCoreCommon ../../engineTest/TestCoreCommon/*.o -lpthread -Wl,-rpath=./ -L. -lBaseCommon -lBaseNetwork -lCoreCommon -ltinyxml2
	
.PHONY: c_obj
c_obj: $(C_OBJS)

.PHONY: cxx_obj
cxx_obj: $(CXX_OBJS)

.PHONY: clean
clean:
	-rm -f *.o
	-rm -f $(BIN_NAME)