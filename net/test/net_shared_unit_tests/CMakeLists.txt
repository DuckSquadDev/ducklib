cmake_minimum_required(VERSION 3.31)

project(ducklib-net-unit-tests)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/f8d7d77c06936315286eb55f8de22cd23c188571.zip
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(
    ${PROJECT_NAME}
    main.cpp
)

target_compile_options(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Debug>:/Zi /Od>
        $<$<CONFIG:RelWithDebInfo>:/Zi /O2>
        $<$<CONFIG:Release>:/O2 /DNDEBUG>
)
target_include_directories(${PROJECT_NAME} PRIVATE ../../../include)
link_directories(${CMAKE_SOURCE_DIR}/bin)
target_link_libraries(
    ${PROJECT_NAME}
    GTest::gtest_main
    ducklib-net
    Ws2_32
)

include(GoogleTest)
gtest_discover_tests(${PROJECT_NAME})