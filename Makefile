$(shell if [ ! -d "bin" ]; then mkdir "bin"; fi)
$(shell if [ ! -d "bin/linux" ]; then mkdir "bin/linux"; fi)
export BIN_ROOT_DIR	:= $(shell pwd)/bin/linux

ifeq ($(ver), debug)
	export COMPILE_OPTION=-O0 -D _DEBUG
else
	export COMPILE_OPTION=-O3
endif

.PHONY: engine
engine:
	make -C engine/src/libBaseCommon;
	make -C engine/src/libBaseNetwork;
	make -C engine/src/libBaseDatabase;
	make -C engine/src/libBaseLua;
	make -C engine/src/libCoreCommon;
	
.PHONY: engineTest
engineTest:
	make -C engineTest/TestBaseDatabase;
	make -C engineTest/TestBaseNetwork;
	make -C engineTest/TestCoreCommon;
	make -C engineTest/TestBaseLua;
	
.PHONY: clean_engine
clean_engine:
	make -C engine/src/libBaseCommon clean;
	make -C engine/src/libBaseNetwork clean;
	make -C engine/src/libBaseDatabase clean;
	make -C engine/src/libBaseLua clean;
	make -C engine/src/libCoreCommon clean;
	
.PHONY: clean_engineTest
clean_engineTest:
	make -C engineTest/TestBaseDatabase clean;
	make -C engineTest/TestBaseNetwork clean;
	make -C engineTest/TestCoreCommon clean;
	make -C engineTest/TestBaseLua clean;