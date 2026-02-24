cmake_minimum_required(VERSION 3.16)

# 明確使用 g++ 作為 C++ 編譯器
if(NOT DEFINED CMAKE_CXX_COMPILER)
    set(CMAKE_CXX_COMPILER g++)
endif()

project(thread-pool)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(thread_pool main.cpp)
