cmake_minimum_required(VERSION 2.6)
project(mousefix)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -lstdc++fs")
add_executable(mousefix main.cpp)

install(TARGETS mousefix RUNTIME DESTINATION bin)
