git clone https://github.com/LuaDist/lua.git ./lua_src

cd lua_src
git checkout 5.3.2

cmake -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=Debug -DBUILD_SHARED_LIBS=ON -DCMAKE_INSTALL_PREFIX=./install_debug ./
nmake
nmake install
xcopy lua.pdb install_debug\bin /y

cmake -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=ON -DCMAKE_INSTALL_PREFIX=./install_release ./
nmake
nmake install
xcopy lua.pdb install_release\bin /y

cd ../
mkdir include & cd include
mkdir lua-5.3.2 & cd ..
mkdir lib & cd lib
mkdir win32 & cd win32
mkdir debug
mkdir release

cd ../../

xcopy lua_src\install_debug\include include\lua-5.3.2 /e /s /i /y

xcopy lua_src\install_debug\lib\lua.lib lib\win32\debug /y
xcopy lua_src\install_debug\bin\lua.dll lib\win32\debug /y
xcopy lua_src\install_debug\bin\lua.pdb lib\win32\debug /y

xcopy lua_src\install_release\lib\lua.lib lib\win32\release /y
xcopy lua_src\install_release\bin\lua.dll lib\win32\release /y
xcopy lua_src\install_release\bin\lua.pdb lib\win32\release /y