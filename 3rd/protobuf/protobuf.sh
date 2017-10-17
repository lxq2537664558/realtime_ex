#!/bin/sh

git clone https://github.com/google/protobuf.git ./protobuf_src
cd protobuf_src
git checkout v3.3.1

cd cmake
mkdir -p build
cd build

cmake -DCMAKE_C_FLAGS="${CMAKE_C_FLAGS} -fPIC"  -DCMAKE_CXX_FLAGS="${CMAKE_CXX_FLAGS} -fPIC" -DCMAKE_BUILD_TYPE=Release -Dprotobuf_BUILD_TESTS=OFF -Dprotobuf_BUILD_SHARED_LIBS=OFF -DCMAKE_INSTALL_PREFIX=../../install ../
make
make install

cd ../../../
mkdir -p include
mkdir -p lib
mkdir -p lib/linux

cp -rf protobuf_src/install/include ./
cp -rf protobuf_src/install/lib64/libprotobufd.a lib/linux/libprotobuf.a