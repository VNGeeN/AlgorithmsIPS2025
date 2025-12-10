# В строке текстового файла  задано  выражение  из  целых 
# чисел и операций '+', '-', '*', '/', '^', SIN, COS, EXP. Порядок
# вычислений  определяется  приоритетом  операций   и   круглыми
# скобками. Возможен одноместный минус в  начале  выражения  или
# после открывающей скобки. Преобразовать выражение в постфиксную
# форму (алгоритм Дейкстры) и вычислить его  значение.  Показать 
# этапы  выполнения (11).

# Автор: Винокуренко Н.Г.

# Среда выполнения: gcc version 9.4.0 (Ubuntu 9.4.0-1ubuntu1~20.04.2) 

# Источники:
# https://habr.com/ru/articles/111361/ 
# https://habr.com/ru/articles/856166/
# https://en.cppreference.com/w/cpp/filesystem.html (C++ Reference)
# https://www.cppstories.com/2024/common-filesystem-cpp20/ (C++ Stories)
# https://www.studyplan.dev/pro-cpp/file-system 

cmake_minimum_required(VERSION 3.0)
project(AlgosIPS2025 VERSION 1.0.0 LANGUAGES CXX)

# Настройка стандарта C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Глобальные флаги компиляции
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fnon-call-exceptions")
endif()

# Include directories
include_directories(include)

# Создаем библиотеку
add_library(ExpressionParser STATIC 
    src/lib/ExpressionParser.cpp
)

# Включаем директории с заголовками
target_include_directories(ExpressionParser PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

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

add_executable(error_tests examples/error_tests.cpp)
target_link_libraries(error_tests ExpressionParser)

# Настройки для отладки
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0")
endif()