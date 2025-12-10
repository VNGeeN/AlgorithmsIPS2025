cmake_minimum_required(VERSION 3.8 FATAL_ERROR)
project(ElectricScheme)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/ElectricSchemeAnalyzer)

set(SOURCES
    main.cpp
    ElectricSchemeAnalyzer/GraphAnalyzer.cpp
    ElectricSchemeAnalyzer/CircuitFinder.cpp
    ElectricSchemeAnalyzer/FileHandler.cpp
    ElectricSchemeAnalyzer/UserInterface.cpp
)

set(HEADERS
    ElectricSchemeAnalyzer/Edge.h
    ElectricSchemeAnalyzer/ShortCircuit.h
    ElectricSchemeAnalyzer/FileHandler.h
    ElectricSchemeAnalyzer/CircuitFinder.h
    ElectricSchemeAnalyzer/GraphAnalyzer.h
    ElectricSchemeAnalyzer/UserInterface.h
)

add_executable(ElectricSchemeAnalyzer ${SOURCES} ${HEADERS})