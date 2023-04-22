cmake_minimum_required(VERSION 3.20)
project("CMU15-213")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake/modules")
include(add_targets)

add_subdirectory("pv")