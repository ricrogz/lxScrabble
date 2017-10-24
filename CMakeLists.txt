cmake_minimum_required(VERSION 2.5)
project(lxScrabble)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -Wall")

# add_definitions(-DCHEAT)
# add_definitions(-DOFFLINE)

# Statically link libraries
SET(CMAKE_FIND_LIBRARY_SUFFIXES ".a")

set(SOURCE_FILES
        src/lxScrabble.cpp
        src/dict_handler.cpp
        src/scores_handler.cpp
        src/irc.cpp
        src/game.cpp
        src/bot_commands.cpp
        src/mimics.h
        src/version.h)

add_executable(lxScrabble ${SOURCE_FILES})


#################################

find_package(Curses REQUIRED)
include_directories(${CURSES_INCLUDE_DIRS})
target_link_libraries(lxScrabble ${CURSES_LIBRARY_DIR})

#################################

include_directories(dependencies/inicpp/include)
target_link_libraries(lxScrabble inicpp_static)
add_subdirectory(dependencies/inicpp)

#################################