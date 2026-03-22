cmake_minimum_required(VERSION 3.10)
project(plotter)

set(CMAKE_CXX_STANDARD 17)

find_package(SDL2 REQUIRED)

add_executable(plotter ../plotter/plot.cpp)

target_link_libraries(plotter SDL2::SDL2)