add_executable(components-example main.cpp)
target_link_libraries(components-example minimeta)
target_include_directories(components-example PUBLIC ${PROJECT_SOURCE_DIR}/include)