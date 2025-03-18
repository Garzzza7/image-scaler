#!/bin/bash

if command -v g++ 2>&1 >/dev/null
then
    echo "g++ detected!!!"
    g++ -g -Ofast -Wall -pedantic -std=c++0x -pthread -static src/main.cpp -o image-scaler
elif command -v clang++ 2>&1 >/dev/null
then
    echo "clang++ detected!!!"
    clang++ -g -Ofast -Wall -pedantic -std=c++0x -pthread -static src/main.cpp -o image-scaler
elif command -v zig 2>&1 >/dev/null
then
    echo "zig detected!!!"
    zig c++ -g -Ofast -Wall -pedantic -std=c++0x -pthread -static src/main.cpp -o image-scaler
else

    echo "No compiler detected!!!"
fi
