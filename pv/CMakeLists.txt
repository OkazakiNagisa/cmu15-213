cmake_minimum_required(VERSION 3.20)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 20)
include_directories("${CMAKE_CURRENT_LIST_DIR}/include")

add_targets(SCAN_SINGLE_FILE TARGET_PREFIX pv. SSF_PATTERN *.single.c)