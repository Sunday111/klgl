cmake_minimum_required(VERSION 3.20)
include(set_compiler_options)
set(module_source_files
    ${CMAKE_CURRENT_SOURCE_DIR}/code/private/main.cpp)
add_executable(two_textures ${module_source_files})
set_generic_compiler_options(two_textures PRIVATE)
target_link_libraries(two_textures PUBLIC klgl)
target_include_directories(two_textures PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/code/public)
target_include_directories(two_textures PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/code/private)
add_custom_command(TARGET two_textures
    POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_CURRENT_SOURCE_DIR}/content"
    $<TARGET_FILE_DIR:two_textures>/content)
