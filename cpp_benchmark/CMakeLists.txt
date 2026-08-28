cmake_minimum_required(VERSION 3.10)
project(CppBench CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS_RELEASE "-O3")

find_package(Threads REQUIRED)

add_executable(cppbench src/main.cpp)
target_link_libraries(cppbench PRIVATE Threads::Threads)
