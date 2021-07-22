set(LLVM_LINK_COMPONENTS support)

add_clang_executable(minimeta
  minimeta.cpp
)

target_link_libraries(minimeta
  PRIVATE
  clangTooling
  clangBasic
  clangASTMatchers
)