# cmake_minimum_required(VERSION <specify CMake version here>)
project(wscrabble)

set(CMAKE_CXX_STANDARD 11)

#set(SOURCE_FILES
#        port.h
#        WScrabble.cpp neWScrabble.cpp)
#
#add_executable(wscrabble ${SOURCE_FILES})

set(NEW_SOURCE_FILES
        src/neWScrabble.cpp
        src/dict_handler.cpp
        src/scores_handler.cpp
        ./lib/inih/cpp/INIReader.cpp
        ./lib/inih/ini.c src/neWScrabble.h src/scores_handler.cpp src/scores_handler.h src/dict_handler.cpp src/dict_handler.h)


add_executable(neWScrabble ${NEW_SOURCE_FILES})