cmake_minimum_required(VERSION 3.20)
include(set_compiler_options)
set(module_source_files
    ${CMAKE_CURRENT_SOURCE_DIR}/code/private/curve_example.cpp)
add_executable(klgl_curve_example ${module_source_files})
set_generic_compiler_options(klgl_curve_example PRIVATE)
target_link_libraries(klgl_curve_example PUBLIC klgl)
target_include_directories(klgl_curve_example PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/code/public)
target_include_directories(klgl_curve_example PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/code/private)
add_custom_target(klgl_curve_example_copy_files ALL
    ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/content" ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/content
)
add_dependencies(klgl_curve_example_copy_files klgl_curve_example)
