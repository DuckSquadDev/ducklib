cmake_minimum_required(VERSION 3.31)

project(ducklib-render-test-hello-world)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(${PROJECT_NAME} WIN32
        render_hello_world.cpp)

target_include_directories(${PROJECT_NAME} PRIVATE ../../../include)
link_directories(${CMAKE_SOURCE_DIR}/dir)
target_link_libraries(${PROJECT_NAME}
        ducklib-render)

set(SOURCE_FILE "${CMAKE_SOURCE_DIR}shaders.hlsl")

add_custom_command(
        OUTPUT ${DESTINATION_DIR}shaders.hlsl
        COMMAND ${CMAKE_COMMAND} -E copy ${SOURCE_FILE} ${DESTINATION_DIR}shaders.hlsl
        DEPENDS ${SOURCE_FILE}
        COMMENT "Copying ${SOURCE_FILE} to ${DESTINATION_DIR}")