cmake_minimum_required(VERSION 3.16)

project(Server LANGUAGES C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(server
    server.cpp
    ../include/net.cpp
    ../include/wordlist.c
)

target_include_directories(server
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

target_compile_options(server
    PRIVATE
        $<$<COMPILE_LANGUAGE:C>:-Wall -Wextra>
        $<$<COMPILE_LANGUAGE:CXX>:-Wall -Wextra>
)
