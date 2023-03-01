cmake_minimum_required(VERSION 3.20)
project("unx_algorithm")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake/Modules")

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 20)
include(MSVCPermissiveBehavior)
include_directories("${CMAKE_CURRENT_LIST_DIR}/include")

add_subdirectory("pv")