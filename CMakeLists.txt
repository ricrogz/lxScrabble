# cmake_minimum_required(VERSION <specify CMake version here>)
project(wscrabble)

set(CMAKE_CXX_STANDARD 11)

#set(SOURCE_FILES
#        port.h
#        WScrabble.cpp neWScrabble.cpp)
#
#add_executable(wscrabble ${SOURCE_FILES})

set(NEW_SOURCE_FILES
        neWScrabble.cpp
        ./lib/inih/cpp/INIReader.cpp
        ./lib/inih/ini.c neWScrabble.h)


add_executable(neWScrabble ${NEW_SOURCE_FILES})