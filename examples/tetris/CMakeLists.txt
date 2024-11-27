cmake_minimum_required(VERSION 3.20)
include(set_compiler_options)
set(module_source_files
    ${CMAKE_CURRENT_SOURCE_DIR}/code/private/prefabs.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/code/private/tetris_example.cpp)
add_executable(klgl_tetris_example ${module_source_files})
set_generic_compiler_options(klgl_tetris_example PRIVATE)
target_link_libraries(klgl_tetris_example PUBLIC klgl)
target_include_directories(klgl_tetris_example PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/code/public)
target_include_directories(klgl_tetris_example PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/code/private)
