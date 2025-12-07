cmake_minimum_required(VERSION 3.31)

project(ducklib-net-unit-tests)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.17.0.zip
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)
enable_testing()

add_executable(
        ${PROJECT_NAME}
        address_tests.cpp
        serialization_tests.cpp
)

target_compile_options(${PROJECT_NAME} PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/W4>
)
target_include_directories(${PROJECT_NAME} PRIVATE ../../../include)
target_link_libraries(${PROJECT_NAME}
        GTest::gtest_main
        ducklib-net
        Ws2_32
)

include(GoogleTest)
gtest_discover_tests(${PROJECT_NAME})