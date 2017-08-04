git clone -b 5.0.0 https://github.com/leethomason/tinyxml2.git ./tinyxml2_src

cd tinyxml2_src
cmake -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=Debug -DBUILD_SHARED_LIBS=ON -DCMAKE_INSTALL_PREFIX=./install_debug ./
nmake
nmake install

cd ../
mkdir include
mkdir include/tinyxml2
mkdir lib & cd lib
mkdir win32 & cd win32
mkdir debug
mkdir release

cd ../../

xcopy tinyxml2_src\install_debug\include include\tinyxml2 /e /s /i /y

xcopy tinyxml2_src\install_debug\lib\tinyxml2d.lib lib\win32\debug /y
xcopy tinyxml2_src\install_debug\bin\tinyxml2d.dll lib\win32\debug /y
xcopy tinyxml2_src\tinyxml2d.pdb lib\win32\debug /y