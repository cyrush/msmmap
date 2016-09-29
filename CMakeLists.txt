cmake_minimum_required (VERSION 2.8.0)
project (msmmap)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_executable(msmmap msmmap.cpp)

enable_testing()

add_test(msmmap_test msmmap)
 