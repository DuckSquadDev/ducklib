cmake_minimum_required(VERSION 3.31)
project(ducklib)

add_subdirectory(core)

add_subdirectory(net)
add_subdirectory(net/test/net_shared_unit_tests)

add_subdirectory(render)

#add_subdirectory(threading)
