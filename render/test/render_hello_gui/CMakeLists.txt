cmake_minimum_required(VERSION 3.31)

project(ducklib-render-test-hello-gui)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(${PROJECT_NAME} WIN32
        render_hello_gui.cpp)

target_compile_options(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Debug>:/Zi /Od>
        $<$<CONFIG:RelWithDebInfo>:/Zi /O2>
        $<$<CONFIG:Release>:/O2 /DNDEBUG>
)
target_include_directories(${PROJECT_NAME} PRIVATE ../../../include)
link_directories(${CMAKE_SOURCE_DIR}/dir)
target_link_libraries(${PROJECT_NAME}
        ducklib-render)

set(SOURCE_FILE "${CMAKE_SOURCE_DIR}shaders.hlsl")

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/shaders.hlsl"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/shaders.hlsl"
        COMMENT "Copying shaders.hlsl to executable directory")
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/gui_shaders.hlsl"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/gui_shaders.hlsl"
        COMMENT "Copying gui_shaders.hlsl to executable directory")
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/text_shaders.hlsl"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/text_shaders.hlsl"
        COMMENT "Copying text_shaders.hlsl to executable directory")