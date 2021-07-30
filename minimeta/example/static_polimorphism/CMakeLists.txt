add_executable(spm-example main.cpp)
target_link_libraries(spm-example minimeta)
target_include_directories(spm-example PUBLIC ${PROJECT_SOURCE_DIR}/include)