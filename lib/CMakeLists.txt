cmake_minimum_required(VERSION 3.10)
project(dx11)

add_library(dx11 code.cpp)
target_include_directories(dx11 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
install(TARGETS dx11 DESTINATION lib)
