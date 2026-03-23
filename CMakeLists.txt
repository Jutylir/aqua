cmake_minimum_required(VERSION 3.10)

project(Aqua)

set(CMAKE_CXX_STANDARD 20)

add_executable(aqua 
./src/main.c 
./src/generator.c
./src/structs.c
./src/tokenizer.c
)