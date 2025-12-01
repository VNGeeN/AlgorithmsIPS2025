cmake_minimum_required(VERSION 3.8 FATAL_ERROR)
project(FileSystemProject)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(FileSystem)

set(SOURCES
    main.cpp
    FileSystem/FileSystem.cpp
    FileSystem/FileSystem.h
)

add_executable(FileSystem ${SOURCES})