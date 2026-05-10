find_package(SDL3     REQUIRED CONFIG)
find_package(SDL3_ttf REQUIRED CONFIG)

add_executable(clock main.cpp)

target_link_libraries(clock PRIVATE ICueNexus SDL3::SDL3 SDL3_ttf::SDL3_ttf)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/font.ttf
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
