#!/bin/bash

rm -rf build/
mkdir build
cd build
cmake ..
make
./image-scaler ../images/butterfly.png 2xbutterfly.png
# open 2xbutterfly.png
