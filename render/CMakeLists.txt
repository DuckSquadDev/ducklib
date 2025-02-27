cmake_minimum_required(VERSION 3.31)

project(ducklib-render)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(${PROJECT_NAME} STATIC
        ../include/render/rhi/swap_chain.h
        src/rhi/rhi.cpp
        ../include/render/rhi/rhi.h
        ../include/render/rhi/device.h
        src/rhi/device.cpp
        ../include/render/rhi/constants.h
        ../include/render/rhi/resources/swap_chain.h
        ../include/render/rhi/resources/descriptor_set.h
        ../include/render/rhi/command_queue.h
        ../include/render/rhi/command_list.h
        src/rhi/command_queue.cpp
        ../include/render/rhi/resources/vertex_buffer.h
        ../include/render/rhi/types.h)

target_include_directories(${PROJECT_NAME} PRIVATE ../include)
set_target_properties(${PROJECT_NAME} PROPERTIES
        PREFIX ""
        OUTPUT_NAME ${PROJECT_NAME}
        SUFFIX ".lib")

add_subdirectory(test/render_hello_world)