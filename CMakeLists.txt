cmake_minimum_required (VERSION 3.16)
project(AhmedLab)
set(CMAKE_CXX_STANDARD 11)
set(src_files ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
              ${CMAKE_CURRENT_SOURCE_DIR}/AhmedLab.cpp
              ${CMAKE_CURRENT_SOURCE_DIR}/Gedo.cpp
              ${CMAKE_CURRENT_SOURCE_DIR}/Gedo.h)
add_executable (AhmedLab ${src_files})
target_link_libraries(AhmedLab PRIVATE $<$<PLATFORM_ID:Linux>:uuid>)
