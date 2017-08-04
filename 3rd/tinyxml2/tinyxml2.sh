#!/bin/sh

git clone -b 5.0.0 https://github.com/leethomason/tinyxml2.git ./tinyxml2_src

cd tinyxml2_src

cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=ON -DCMAKE_INSTALL_PREFIX=./install ./
make
make install

cd ../
mkdir -p include
mkdir -p include/tinyxml2
mkdir -p lib
mkdir -p lib/linux

cp -rf tinyxml2_src/install/include/* include/tinyxml2
cp -rf tinyxml2_src/install/lib64/libtinyxml2.so.5.0.0 lib/linux/libtinyxml2.so