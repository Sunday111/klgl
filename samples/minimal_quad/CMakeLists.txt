cmake_minimum_required(VERSION 3.20)
include(set_compiler_options)
set(module_source_files
    ${CMAKE_CURRENT_SOURCE_DIR}/code/private/main.cpp)
add_executable(klgl_minimal_quad ${module_source_files})
set_generic_compiler_options(klgl_minimal_quad PRIVATE)
target_link_libraries(klgl_minimal_quad PUBLIC klgl)
target_include_directories(klgl_minimal_quad PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/code/public)
target_include_directories(klgl_minimal_quad PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/code/private)
add_custom_command(TARGET klgl_minimal_quad
    POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_CURRENT_SOURCE_DIR}/content"
    $<TARGET_FILE_DIR:klgl_minimal_quad>/content)
