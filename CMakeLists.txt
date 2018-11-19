cmake_minimum_required(VERSION 3.8)
project(Pool)

set(CMAKE_CXX_STANDARD 17)
SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -stdlib=libc++")


set(SOURCE_FILES main.cpp)
add_executable(Pool ${SOURCE_FILES})