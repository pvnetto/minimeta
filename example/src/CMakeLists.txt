add_executable(minimeta-example main.cpp)

target_link_libraries(minimeta-example minimeta)

target_include_directories(minimeta-example
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/vendor/minimeta)

option(RUN_MMETA "Run minimeta before building" OFF)

# Uses /std:c++17 flag when compiling MSVC std library with clang, otherwise it should use -std=c++17
# TODO: Add -std=c++17 when compiling with other platforms
if(RUN_MMETA)
    add_custom_target(
        generate_meta
        COMMAND cmd /c "minimeta ../main.cpp -p ../out/build/x64-Debug/compile_commands.json --extra-arg /std:c++17"
    )
    add_dependencies(minimeta-example generate_meta)
endif()