cmake_minimum_required(VERSION 3.10)

project(TestProject)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -Wall")

include_directories(src)
set(SERVER_SOURCES src/server/main.cpp src/server/server.cpp src/crc32.cpp)
set(CLIENT_SOURCES src/client/main.cpp src/client/client.cpp src/crc32.cpp)

add_executable(server ${SERVER_SOURCES})
add_executable(client ${CLIENT_SOURCES})

add_custom_target(run COMMAND "./server")