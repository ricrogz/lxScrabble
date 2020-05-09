cmake_minimum_required(VERSION 3.5)
project(lxScrabble)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -funsigned-char -fstack-protector-strong -Wl,-z,defs -Wl,-z,now -Wl,-z,relro")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -s")

# For debugging purposes
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic -Wextra")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
# target_add_definitions(lxScrabble -DCHEAT)

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
# inicpp dependency

set(BUILD_STATIC OFF)
set(BUILD_SHARED ON)

include_directories(dependencies/inicpp/include)
target_link_libraries(lxScrabble inicpp)
add_subdirectory(dependencies/inicpp)

#################################
# fmtlib dependency

find_package(fmt)

if(NOT fmt_FOUND)
    set(BUILD_SHARED_LIBS ON)

    find_package(Git REQUIRED)

    execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
        dependencies/fmt WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

    include_directories(dependencies/fmt/include)
    add_subdirectory(dependencies/fmt)

endif()

target_link_libraries(lxScrabble fmt::fmt)