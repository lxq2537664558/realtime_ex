git clone -b v3.3.2 https://github.com/google/protobuf.git ./protobuf_src

cd protobuf_src/cmake
mkdir build & cd build
mkdir debug & cd debug
cmake -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=Debug -Dprotobuf_BUILD_TESTS=OFF -Dprotobuf_BUILD_SHARED_LIBS=ON -DCMAKE_INSTALL_PREFIX=../../../install_debug ../..
nmake
nmake install

cd ../../../../
mkdir include
mkdir bin
mkdir lib & cd lib
mkdir win32 & cd win32
mkdir debug
mkdir release

cd ../../

xcopy protobuf_src\install_debug\include include /e /s /i /y

xcopy protobuf_src\install_debug\lib\libprotobufd.lib lib\win32\debug /e /s /i /y
xcopy protobuf_src\install_debug\bin\libprotobufd.dll lib\win32\debug /e /s /i /y
xcopy protobuf_src\cmake\build\debug\libprotobufd.pdb lib\win32\debug /y
xcopy protobuf_src\install_debug\bin bin /e /s /i /y