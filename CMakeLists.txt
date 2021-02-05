cmake_minimum_required (VERSION 3.16)
project(AhmedLab)
set(CMAKE_CXX_STANDARD 11)
set(src_files ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
              ${CMAKE_CURRENT_SOURCE_DIR}/AhmedLab.cpp
              ${CMAKE_CURRENT_SOURCE_DIR}/AhmedLab.h)
add_executable (AhmedLab ${src_files})
