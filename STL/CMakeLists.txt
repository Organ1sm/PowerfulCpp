cmake_minimum_required(VERSION 3.18)
set(CMAKE_CXX_STANDARD 20)

project(STL LANGUAGES CXX)

file(GLOB sources CONFIGURE_DEPENDS *.cpp)
foreach(source IN ITEMS ${sources})
    get_filename_component(name "${source}" NAME_WLE)
    add_executable(${name} ${name}.cpp)
endforeach()
