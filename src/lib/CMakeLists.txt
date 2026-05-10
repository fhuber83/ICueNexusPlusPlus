add_library(ICueNexus STATIC ICueNexus.cpp)

target_include_directories(ICueNexus PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(ICueNexus PUBLIC hidapi-hidraw)
