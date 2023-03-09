cmake_minimum_required(VERSION 3.20)
project("CMU15-213")

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 20)
include_directories("${CMAKE_CURRENT_LIST_DIR}/include")

add_subdirectory("pv")