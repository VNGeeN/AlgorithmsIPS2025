cmake_minimum_required(VERSION 3.8 FATAL_ERROR)
project(lab4)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(lab4)

set(SOURCES
    main.cpp
)

add_executable(lab4 ${SOURCES})