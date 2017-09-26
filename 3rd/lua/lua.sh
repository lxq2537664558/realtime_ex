#!/bin/sh

git clone https://github.com/LuaDist/lua.git ./lua_src

cd lua_src
git checkout 5.3.2

cmake -DCMAKE_BUILD_TYPE=Debug -DBUILD_SHARED_LIBS=ON -DCMAKE_INSTALL_PREFIX=./install ./
make
make install

cd ../
mkdir -p include
mkdir -p include/lua-5.3.2
mkdir -p lib
mkdir -p lib/linux

cp -rf lua_src/install/include/* include/lua-5.3.2
cp -rf lua_src/install/lib/liblua.so lib/linux/liblua.so