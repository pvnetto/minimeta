cmake_minimum_required (VERSION 3.10)
project (minimeta VERSION 0.1
        DESCRIPTION "Runtime static library for Minimeta"
        LANGUAGES CXX)

add_library(minimeta INTERFACE)
add_library(minimeta::minimeta ALIAS minimeta)
target_compile_features(minimeta INTERFACE cxx_std_17)
target_include_directories(minimeta INTERFACE "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
                                                "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>")

target_link_libraries(minimeta INTERFACE yaml-cpp)

add_subdirectory(vendor)

option(MMETA_BUILD_EXAMPLES "Build examples along with library" ON)
if(MMETA_BUILD_EXAMPLES)
    add_subdirectory(example)
endif()