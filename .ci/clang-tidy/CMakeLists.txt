cmake_minimum_required(VERSION 3.20)
project(CustomTidyPlugin LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(LLVM_DIR "/usr/lib/llvm-22/cmake" CACHE PATH "")
set(Clang_DIR "/usr/lib/cmake/clang-22" CACHE PATH "")

find_package(LLVM REQUIRED CONFIG)
find_package(Clang REQUIRED CONFIG)

include_directories(SYSTEM ${LLVM_INCLUDE_DIRS} ${CLANG_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

add_library(CustomTidyPlugin MODULE UseAutoMakeSharedCheck.cpp)
