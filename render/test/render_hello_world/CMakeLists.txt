cmake_minimum_required(VERSION 3.31)

project(ducklib-render-test-hello-world)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(${PROJECT_NAME} WIN32
        render_hello_world.cpp)

target_include_directories(${PROJECT_NAME} PRIVATE ../../../include)
link_directories(${CMAKE_SOURCE_DIR}/dir)
target_link_libraries(${PROJECT_NAME}
        ducklib-core
        ducklib-render
        d3d12
        dxgi)