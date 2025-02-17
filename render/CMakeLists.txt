cmake_minimum_required(VERSION 3.31)

project(ducklib-render)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(${PROJECT_NAME} STATIC
        ../include/render/core/Rhi.h
        ../include/render/core/swap_chain.h
        src/core/d3d12/d3d12_rhi.cpp
        ../include/render/core/d3d12/d3d12_rhi.h
        ../include/render/core/device.h
        ../include/render/core/d3d12/d3d12_device.h
        src/core/D3D12/d3d12_device.cpp
        ../include/render/core/format.h
        ../include/render/core/d3d12/d3d12_format.h
        ../include/render/core/resources/swap_chain.h
        ../include/render/core/d3d12/resources/d3d12_swap_chain.h
        ../include/render/core/descriptor_set.h
        ../include/render/core/d3d12/resources/d3d12_descriptor_set.h
        ../include/render/core/d3d12/d3d12_utility.h)

target_include_directories(${PROJECT_NAME} PRIVATE ../include)
set_target_properties(${PROJECT_NAME} PROPERTIES
        PREFIX ""
        OUTPUT_NAME ${PROJECT_NAME}
        SUFFIX ".lib")

add_subdirectory(test/render_hello_world)