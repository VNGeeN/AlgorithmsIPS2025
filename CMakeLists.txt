cmake_minimum_required(VERSION 3.0)
project(AlgosIPS2025 VERSION 1.0.0 LANGUAGES CXX)

# Настройка стандарта C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Создаем библиотеку
add_library(ExpressionParser STATIC 
    src/lib/ExpressionParser.cpp
)

# Включаем директории с заголовками
target_include_directories(ExpressionParser PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Флаги компиляции для библиотеки
target_compile_options(ExpressionParser PRIVATE -Wall -Wextra -Wpedantic)

# Основное приложение
add_executable(${PROJECT_NAME} 
    app/main.cpp
)

# Линкуем библиотеку с приложением
target_link_libraries(${PROJECT_NAME} ExpressionParser)

# Примеры использования
add_executable(example1 examples/example1.cpp)
target_link_libraries(example1 ExpressionParser)

add_executable(example2 examples/example2.cpp)
target_link_libraries(example2 ExpressionParser)

# Настройки для отладки
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(ExpressionParser PRIVATE -g -O0)
    target_compile_options(${PROJECT_NAME} PRIVATE -g -O0)
endif()