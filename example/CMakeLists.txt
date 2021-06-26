cmake_minimum_required (VERSION 3.6)
project (minimeta-example CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_compile_definitions(__MMETA__)

add_executable(minimeta-example main.cpp)