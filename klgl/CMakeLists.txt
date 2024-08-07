cmake_minimum_required(VERSION 3.20)
include(set_compiler_options)
set(module_source_files
    ${CMAKE_CURRENT_SOURCE_DIR}/code/private/application.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/code/private/events/event_manager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/code/private/filesystem/filesystem.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/code/private/memory/type_erased_array.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/code/private/mesh/mesh_data.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/code/private/mesh/procedural_mesh_generator.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/code/private/name_cache/name.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/code/private/name_cache/name_cache.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/code/private/opengl/debug/annotations.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/code/private/opengl/debug/gl_debug_messenger.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/code/private/opengl/gl_api.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/code/private/platform/glfw/glfw_state.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/code/private/platform/os/os.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/code/private/reflection/reflection_utils.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/code/private/shader/shader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/code/private/shader/shader_define.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/code/private/shader/shader_uniform.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/code/private/texture/texture.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/code/private/ui/type_id_widget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/code/private/window.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/code/public/klgl/application.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/code/public/klgl/error_handling.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/code/public/klgl/events/detail.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/code/public/klgl/events/event_listener.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/code/public/klgl/events/event_listener_interface.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/code/public/klgl/events/event_listener_method.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/code/public/klgl/events/event_manager.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/code/public/klgl/events/mouse_events.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/code/public/klgl/events/window_events.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/code/public/klgl/filesystem/filesystem.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/code/public/klgl/macro/warning_suppress.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/code/public/klgl/memory/type_erased_array.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/code/public/klgl/memory/type_erased_array_adapter.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/code/public/klgl/mesh/mesh_data.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/code/public/klgl/mesh/procedural_mesh_generator.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/code/public/klgl/name_cache/name.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/code/public/klgl/name_cache/name_cache.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/code/public/klgl/opengl/debug/annotations.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/code/public/klgl/opengl/debug/gl_debug_messenger.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/code/public/klgl/opengl/gl_api.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/code/public/klgl/platform/os/os.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/code/public/klgl/reflection/matrix_reflect.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/code/public/klgl/reflection/reflection_utils.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/code/public/klgl/reflection/register_types.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/code/public/klgl/shader/define_handle.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/code/public/klgl/shader/sampler_uniform.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/code/public/klgl/shader/shader.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/code/public/klgl/shader/shader_define.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/code/public/klgl/shader/shader_uniform.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/code/public/klgl/shader/uniform_handle.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/code/public/klgl/template/class_member_traits.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/code/public/klgl/template/get_enum_underlying.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/code/public/klgl/template/member_offset.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/code/public/klgl/template/on_scope_leave.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/code/public/klgl/template/tagged_id_hash.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/code/public/klgl/template/tuple_type_by_index.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/code/public/klgl/template/type_to_gl_type.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/code/public/klgl/texture/texture.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/code/public/klgl/ui/type_id_widget.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/code/public/klgl/window.hpp)
add_library(klgl STATIC ${module_source_files})
set_generic_compiler_options(klgl PRIVATE)
target_link_libraries(klgl PUBLIC edt
                                  fmt
                                  CppReflection
                                  nlohmann_json
                                  glad
                                  glfw
                                  imgui_glfw_cmake
                                  cpptrace::cpptrace
                                  unordered_dense)
target_include_directories(klgl PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/code/public)
target_include_directories(klgl PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/code/private)
