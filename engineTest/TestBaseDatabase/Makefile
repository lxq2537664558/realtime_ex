INCLUDE_DIR := \
-I../../engine/inc\
-I../../3rd/mysql/include

include ../../build.mk


.PHONY: all
all: c_obj cxx_obj
	cd $(BIN_ROOT_DIR);g++ $(COMPILE_OPTION) -rdynamic -o TestBaseDatabase ../../engineTest/TestBaseDatabase/*.o -lpthread -Wl,-rpath=./ -L. -lBaseCommon -lBaseDatabase -lmysqlclient_r
	
.PHONY: c_obj
c_obj: $(C_OBJS)

.PHONY: cxx_obj
cxx_obj: $(CXX_OBJS)

.PHONY: clean
clean:
	-rm -f *.o
	-rm -f $(BIN_NAME)