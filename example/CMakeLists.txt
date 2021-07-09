cmake_minimum_required (VERSION 3.8)
project (minimeta-example CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)

# Uses /std:c++17 flag when compiling MSVC std library with clang, otherwise it should use -std=c++17
# TODO: Add -std=c++17 when compiling with other platforms

add_executable(minimeta-example main.cpp)

option(RUN_MMETA "Run minimeta before building" OFF)

if(RUN_MMETA)
    add_custom_target(
        generate_meta
        COMMAND cmd /c "minimeta ../main.cpp -p ../out/build/x64-Debug/compile_commands.json --extra-arg /std:c++17"
    )
    add_dependencies(minimeta-example generate_meta)
endif()