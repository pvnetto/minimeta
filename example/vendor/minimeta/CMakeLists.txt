cmake_minimum_required (VERSION 3.10)
project (minimeta VERSION 0.1
        DESCRIPTION "Runtime static library for Minimeta"
        LANGUAGES CXX)

add_library(minimeta INTERFACE)
add_library(minimeta::minimeta ALIAS minimeta)

target_include_directories(minimeta INTERFACE "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
                                                "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>")
# set_target_properties(minimeta PROPERTIES LINKER_LANGUAGE CXX)
