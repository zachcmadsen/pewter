cmake_minimum_required(VERSION 3.27)

project(pewter)

add_subdirectory(src)

include(CTest)
enable_testing()
add_subdirectory(tests)
