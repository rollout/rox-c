#!/bin/bash

# NOTE: before running this file vendor libraries must be built.
# To do that proceed to vendor directory and run ./build-third-party-libs.sh

rm -fr build
mkdir build

cd build
cmake ..
make
ctest --output-on-failure .
