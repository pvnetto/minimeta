cmake_minimum_required (VERSION 3.8)
project (minimeta-example CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)

add_subdirectory(src)
add_subdirectory(vendor)