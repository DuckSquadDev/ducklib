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
        ../include/render/rhi/command_list.h
        ../include/render/rhi/types.h
        ../include/render/resource_manager.h
        src/resource_manager.cpp
        ../include/render/rhi/shader.h
        src/rhi/shader.cpp
        src/rhi/types.cpp)

target_include_directories(${PROJECT_NAME} PRIVATE ../include)
set_target_properties(${PROJECT_NAME} PROPERTIES
        PREFIX ""
        OUTPUT_NAME ${PROJECT_NAME}
        SUFFIX ".lib")
target_link_libraries(${PROJECT_NAME}
        ducklib-core
        d3d12
        dxgi
        dxguid
        d3dcompiler)

add_subdirectory(test/render_hello_world)