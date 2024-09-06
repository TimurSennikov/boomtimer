cmake_minimum_required(VERSION 3.13...3.27)

add_compile_definitions(PICO_EXCEPTIONS=1)
set(PICO_CXX_ENABLE_EXCEPTIONS 1)
include(pico_sdk_import.cmake)

project(boomtimer)

pico_sdk_init()

add_executable(main main.cpp)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fexceptions -fno-rtti -fno-unwind-tables")

target_link_libraries(main pico_stdlib)

pico_add_extra_outputs(main)
