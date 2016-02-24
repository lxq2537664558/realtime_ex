编译lua动态态库时需要修改一下Makefile文件
在CFLAGS中添加-fPIC -g -rdynamic 
并且执行
g++ -g -fPIC -O2 -shared -o liblua.so lapi.o lcode.o ldebug.o ldo.o ldump.o lfunc.o lgc.o llex.o lmem.o lobject.o lopcodes.o lparser.o lstate.o lstring.o ltable.o ltm.o lundump.o lvm.o lzio.o lauxlib.o lbaselib.o ldblib.o liolib.o lmathlib.o loslib.o ltablib.o lstrlib.o loadlib.o linit.o