cmake_minimum_required (VERSION 3.6)
project (minimeta-example CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# TODO: Add a custom target that generates a compilation database before calling generate_meta

add_custom_target(
    generate_meta
    COMMAND cmd /c "minimeta ../../main.cpp -p x64-Debug/compile_commands.json"
)

add_executable(minimeta-example main.cpp)
add_dependencies(minimeta-example generate_meta)