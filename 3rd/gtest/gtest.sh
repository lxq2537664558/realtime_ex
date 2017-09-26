#!/bin/sh

git clone https://github.com/google/googletest.git ./gtest_src

cd gtest_src
git checkout release-1.8.0

cmake -DCMAKE_BUILD_TYPE=Debug -DBUILD_GMOCK=OFF -DBUILD_GTEST=ON -Dgtest_force_shared_crt=ON -DCMAKE_INSTALL_PREFIX=./install ./
make
make install

cd ../
mkdir -p include
mkdir -p lib
mkdir -p lib/linux

cp -rf gtest_src/install/include/gtest include
cp -rf gtest_src/install/lib/libgtest.a lib/linux/libgtest.a