cmake_minimum_required(VERSION 2.5)
project(lxScrabble)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -funsigned-char -fvisibility=hidden -fstack-protector-strong -Wl,-z,defs -Wl,-z,now -Wl,-z,relro")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -s")

# For debugging purposes
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic -Wextra")
#set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
#add_definitions(-DCHEAT)

set(SOURCE_FILES
        src/lxScrabble.cpp
        src/dict_handler.cpp
        src/scoreboard.cpp
        src/irc.cpp
        src/game.cpp
        src/bot_commands.cpp
        src/mimics.hpp
        src/version.hpp)

add_executable(lxScrabble ${SOURCE_FILES})

#################################

include_directories(dependencies/inicpp/include)
target_link_libraries(lxScrabble inicpp_static)
add_subdirectory(dependencies/inicpp)

#################################
