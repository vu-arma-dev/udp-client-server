cmake_minimum_required (VERSION 3.5.1)
project (udp_client_server) 

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(${PROJECT_NAME}
   src/udp_client_server.cpp
)

target_include_directories(udp_client_server PUBLIC include/${PROJECT_NAME})