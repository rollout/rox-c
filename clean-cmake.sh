#!/bin/bash

rm -fr `find . -name CMakeFiles`
rm -f `find . -name CMakeCache.txt`
rm -f `find . -name Makefile`
rm -f `find . -name cmake_install.cmake`
rm -f `find . -name "*.a"`
rm -fr build
mkdir build

cd build
cmake ..
make
ctest --output-on-failure .
