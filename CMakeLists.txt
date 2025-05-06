cmake_minimum_required(VERSION 3.10)
project(FP8Demo)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(fp16 src/FP16.cpp)
