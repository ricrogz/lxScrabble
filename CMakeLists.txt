# cmake_minimum_required(VERSION <specify CMake version here>)
project(wscrabble)

set(CMAKE_CXX_STANDARD 11)

# inih library include files
set(INIH_SOURCE
        ./lib/inih/cpp/INIReader.cpp
        ./lib/inih/ini.c)

set(NEW_SOURCE_FILES
        ${INIH_SOURCE}
        src/lxScrabble.cpp
        src/dict_handler.cpp
        src/scores_handler.cpp
        src/irc.cpp)

add_executable(lxScrabble ${NEW_SOURCE_FILES})