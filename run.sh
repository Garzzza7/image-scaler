#!/bin/bash

rm -rf build/
mkdir build
cd build
cmake ..
make
../images/butterfly.png
./image-scaler ../images/butterfly.png 2xbutterfly.png
open 2xbutterfly.png
open ../images/butterfly.png
