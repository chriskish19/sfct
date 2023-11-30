# Set the minimum required version of CMake to be 3.10
cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

# Create a project named sfct
project(sfct VERSION 1.0.0.0)

# Set the variable CMAKE_CXX_STANDARD to 17
# and the variable CMAKE_CXX_STANDARD_REQUIRED to True
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED true)

add_executable(sfct src/main.cpp)