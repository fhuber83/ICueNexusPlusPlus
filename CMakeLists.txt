cmake_minimum_required(VERSION 4.2)
project(HID_Test)

set(CMAKE_CXX_STANDARD 26)

find_package(SDL3     REQUIRED CONFIG)
find_package(SDL3_ttf REQUIRED CONFIG)

add_executable(HID_Test
        main.cpp
        ICueNexus.cpp)

target_link_libraries(HID_Test PRIVATE hidapi-hidraw)
target_link_libraries(HID_Test PRIVATE SDL3::SDL3 SDL3_ttf::SDL3_ttf)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/font.ttf
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
