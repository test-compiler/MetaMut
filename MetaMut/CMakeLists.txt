cmake_minimum_required (VERSION 3.16)
project(LLMut CXX C)
include(cmake/string_to_list.cmake)
include(cmake/find_llvm.cmake)

set(CMAKE_C_COMPILER "gcc")
set(CMAKE_CXX_COMPILER "g++")

set(LINK_LIBS
  clangTooling
  clangToolingASTDiff
  clangToolingRefactoring
  clangToolingCore
  clangASTMatchers
  clangFrontend
  clangDriver
  clangSerialization
  clangTransformer
  clangRewrite
  clangFormat
  clangToolingInclusions
  clangParse
  clangSema
  clangAnalysis
  clangEdit
  clangAST
  clangLex
  clangBasic
  clang
  LLVM)

file(GLOB_RECURSE SOURCES "lib/*.cpp")
add_executable(LLMut ${SOURCES} main.cpp)
target_include_directories(LLMut PRIVATE include)
target_link_libraries(LLMut PRIVATE ${LINK_LIBS})
target_include_directories(LLMut PRIVATE
  . ${LLVM_INCLUDE_DIRS})
target_link_directories(LLMut PRIVATE
  ${LLVM_LIBRARY_DIRS})
target_compile_options(LLMut PRIVATE -ggdb3 -w -fmax-errors=3)
