cmake_minimum_required(VERSION 3.20)
include(set_compiler_options)
set(module_source_files
    ${CMAKE_CURRENT_SOURCE_DIR}/code/private/painter2d_example.cpp)
add_executable(klgl_painter2d_example ${module_source_files})
set_generic_compiler_options(klgl_painter2d_example PRIVATE)
target_link_libraries(klgl_painter2d_example PUBLIC klgl)
target_include_directories(klgl_painter2d_example PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/code/public)
target_include_directories(klgl_painter2d_example PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/code/private)
