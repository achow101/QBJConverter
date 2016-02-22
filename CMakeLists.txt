cmake_minimum_required(VERSION 3.3)
project(QBJConverter)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES main.cpp json/json-forwards.h json/json.h jsoncpp.cpp)
add_executable(QBJConverter ${SOURCE_FILES})