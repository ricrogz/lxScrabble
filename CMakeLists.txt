# cmake_minimum_required(VERSION <specify CMake version here>)
project(wscrabble)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES
        port.h
        WScrabble.cpp)

add_executable(wscrabble ${SOURCE_FILES})