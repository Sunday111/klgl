cmake_minimum_required(VERSION 3.20)
include(set_compiler_options)
set(module_source_files
    ${CMAKE_CURRENT_SOURCE_DIR}/code/private/minimal_cube_example.cpp)
add_executable(klgl_minimal_cube_example ${module_source_files})
set_generic_compiler_options(klgl_minimal_cube_example PRIVATE)
target_link_libraries(klgl_minimal_cube_example PUBLIC klgl)
target_include_directories(klgl_minimal_cube_example PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/code/public)
target_include_directories(klgl_minimal_cube_example PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/code/private)
add_custom_target(klgl_minimal_cube_example_copy_files ALL
    ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/content" ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/content
)
add_dependencies(klgl_minimal_cube_example_copy_files klgl_minimal_cube_example)
