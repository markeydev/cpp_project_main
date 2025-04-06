cmake_minimum_required(VERSION 3.12)
project(toy_interpreter VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -pedantic)
endif()

include_directories(include)
file(GLOB SOURCES "src/*.cpp")

add_executable(interpreter ${SOURCES})

configure_file(test/test.toy ${CMAKE_BINARY_DIR}/test.toy COPYONLY)