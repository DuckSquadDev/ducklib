cmake_minimum_required(VERSION 3.31)

project(ducklib-core)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(${PROJECT_NAME} STATIC
        ../include/ducklib/core/logging/logger.h
        ../include/ducklib/core/app_window.h
        ../include/ducklib/core/win/win_app_window.h
        src/logger.cpp
        src/win/win_app_window.cpp
        ../include/ducklib/math/math.h
        src/math.cpp
        ../include/ducklib/core/logging/log_level.h
        ../include/ducklib/core/unicode.h
        src/unicode.cpp)

target_compile_options(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Debug>:/Zi /Od>
        $<$<CONFIG:RelWithDebInfo>:/Zi /O2>
        $<$<CONFIG:Release>:/O2 /DNDEBUG>
)
target_include_directories(${PROJECT_NAME} PUBLIC ../include)
set_target_properties(${PROJECT_NAME} PROPERTIES
        PREFIX ""
        OUTPUT_NAME ${PROJECT_NAME}
        SUFFIX ".lib")