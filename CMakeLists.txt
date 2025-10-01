# Корневой CMakeLists.txt
cmake_minimum_required(VERSION 3.10.0)
project(generator VERSION 0.1.0 LANGUAGES C CXX)

# Установка стандарта C++
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Указание исходных файлов
set(SOURCES
    src/main.cpp
    src/functions.cpp
    src/parsing.cpp
)

# Указание заголовочных файлов
set(HEADERS
    include/functions.h
    include/parsing.h
)

# Создание исполняемого файла
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# Включение директории с заголовками
target_include_directories(${PROJECT_NAME} PRIVATE include)

# Установка выходной директории (опционально)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)