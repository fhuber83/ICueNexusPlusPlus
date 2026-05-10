cmake_minimum_required(VERSION 4.2)
project(HID_Test)

set(CMAKE_CXX_STANDARD 26)

add_executable(HID_Test main.cpp)
target_link_libraries(HID_Test PRIVATE hidapi-hidraw)