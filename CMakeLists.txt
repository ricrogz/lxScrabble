# cmake_minimum_required(VERSION <specify CMake version here>)
project(wscrabble)

set(CMAKE_CXX_STANDARD 11)

set(NEW_SOURCE_FILES
        src/lxScrabble.cpp
        src/dict_handler.cpp
        src/scores_handler.cpp
        src/irc.cpp
        ./lib/inih/cpp/INIReader.cpp
        ./lib/inih/ini.c)

add_executable(lxScrabble ${NEW_SOURCE_FILES})