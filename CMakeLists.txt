# cmake_minimum_required(VERSION <specify CMake version here>)
cmake_minimum_required(VERSION 3.8)
project(lxScrabble)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -Wall")

# add_definitions(-DCHEAT)

set(SOURCE_FILES
        src/lxScrabble.cpp
        src/dict_handler.cpp
        src/scores_handler.cpp
        src/irc.cpp
        src/game.cpp
        src/mimics.cpp)

add_executable(lxScrabble ${SOURCE_FILES})

#################################

find_package(Curses REQUIRED)
include_directories(${CURSES_INCLUDE_DIRS})
target_link_libraries(lxScrabble ${CURSES_LIBRARIES})

#################################

include_directories(dependencies/inicpp/include)
target_link_libraries(lxScrabble inicpp)
add_subdirectory(dependencies/inicpp)

#################################