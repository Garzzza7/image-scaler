# emergence generation of complie_commands.json file for
# clangd to parse every used file properly
# cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=1
# the file should be detected from the build directory,
# in case it is not read this https://clangd.llvm.org/installation
# also you might want to check for colliding versions here
# /usr/lib/gcc/x86_64-linux-gnu/
cmake_minimum_required(VERSION 3.22)
set(project_name "image-scaler")

project(${project_name})

add_compile_options(-g)
add_compile_options(-Ofast)
add_compile_options(-Wall)
add_compile_options(-pedantic)
add_compile_options(-std=c++0x)
add_compile_options(-pthread)

add_link_options(-static)

add_executable(${project_name} src/main.cpp)
