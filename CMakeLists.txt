cmake_minimum_required(VERSION 3.23)
project("CMU15-213")

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 20)

add_targets(SCAN_SINGLE_FILE SSF_PATTERN *.single.c SUBDIR pv TARGET_PREFIX pv.)