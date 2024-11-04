cmake_minimum_required (VERSION 3.5.1)
project (udp_client_server) 

set(CMAKE_CXX_STANDARD 11)

# Declare c and C++ Compilers
set(CMAKE_C_COMPILER "gcc")
set(CMAKE_CXX_COMPILER "g++")

# Make sure include directory is included
include_directories(
  include
)

# Declare C++ libraries
add_library(udp_utils src/udp_utils.cpp src/udp_client_server.cpp)
target_include_directories(udp_utils PUBLIC include)

# Declare a C++ Executable
add_executable(udp_example src/udp_example.cpp) 

# Link exe with libraries
target_link_libraries(udp_example udp_utils ws2_32)






