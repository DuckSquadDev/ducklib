cmake_minimum_required(VERSION 3.31)

project(ducklib-render)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(${PROJECT_NAME} STATIC
        ../include/ducklib/render/rhi/swap_chain.h
        src/rhi/rhi.cpp
        ../include/ducklib/render/rhi/rhi.h
        ../include/ducklib/render/rhi/device.h
        src/rhi/device.cpp
        ../include/ducklib/render/rhi/constants.h
        ../include/ducklib/render/rhi/resources/swap_chain.h
        ../include/ducklib/render/rhi/resources/descriptor_set.h
        ../include/ducklib/render/rhi/command_list.h
        ../include/ducklib/render/rhi/types.h
        ../include/ducklib/render/resource_manager.h
        src/resource_manager.cpp
        ../include/ducklib/render/rhi/shader.h
        src/rhi/shader.cpp
        src/rhi/types.cpp
        ../include/ducklib/render/render_util.h
        ../include/ducklib/render/gui/font.h
        src/gui/font.cpp
        src/render_util.cpp
        ../include/ducklib/render/gui/gui.h
        src/gui/gui.cpp)

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
target_link_libraries(${PROJECT_NAME}
        PUBLIC
        ducklib-core
        d3d12
        dxgi
        dxguid
        d3dcompiler)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/src/Roboto-Regular.ttf"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/Roboto-Regular.ttf"
        COMMENT "Copying Roboto-Regular.ttf to executable directory")

add_subdirectory(test/render_hello_triangle)
add_subdirectory(test/render_hello_gui)