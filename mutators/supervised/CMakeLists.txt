cmake_minimum_required (VERSION 3.16)
project(MutatorCollection CXX C)
include(string_to_list.cmake)
include(find_llvm.cmake)

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

file(GLOB_RECURSE SOURCES "*/*.cc")
add_executable(MutatorCollection ${SOURCES} main.cc)
target_include_directories(MutatorCollection PRIVATE include)
target_link_libraries(MutatorCollection PRIVATE ${LINK_LIBS})
target_include_directories(MutatorCollection PRIVATE
  . ${LLVM_INCLUDE_DIRS})
target_link_directories(MutatorCollection PRIVATE
  ${LLVM_LIBRARY_DIRS})
target_compile_options(MutatorCollection PRIVATE -ggdb3)
