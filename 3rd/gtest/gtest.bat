git clone https://github.com/google/googletest.git ./gtest_src

cd gtest_src
git checkout release-1.8.0

cmake -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=Debug -DBUILD_GMOCK=OFF -DBUILD_GTEST=ON -Dgtest_force_shared_crt=ON -DCMAKE_INSTALL_PREFIX=./install_debug ./
nmake
nmake install

cmake -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=Release -DBUILD_GMOCK=OFF -DBUILD_GTEST=ON -Dgtest_force_shared_crt=ON -DCMAKE_INSTALL_PREFIX=./install_release ./
nmake
nmake install

cd ../
mkdir include
mkdir lib & cd lib
mkdir win32 & cd win32
mkdir debug
mkdir release

cd ../../

xcopy gtest_src\install_debug\include\gtest include\gtest /e /s /i /y

xcopy gtest_src\install_debug\lib\gtest.lib lib\win32\debug /y

xcopy gtest_src\install_release\lib\gtest.lib lib\win32\release /y