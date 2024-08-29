cmake_minimum_required(VERSION 3.20)
include(set_compiler_options)
set(module_source_files
    ${CMAKE_CURRENT_SOURCE_DIR}/code/private/rotator_benchmark.cpp)
add_executable(klgl_rotator_benchmark ${module_source_files})
set_generic_compiler_options(klgl_rotator_benchmark PRIVATE)
target_link_libraries(klgl_rotator_benchmark PUBLIC klgl
                                                    benchmark::benchmark_main)
target_include_directories(klgl_rotator_benchmark PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/code/public)
target_include_directories(klgl_rotator_benchmark PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/code/private)
