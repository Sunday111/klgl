#include "klgl/shader/shader.hpp"

#include <fmt/chrono.h>
#include <fmt/std.h>
#include <imgui.h>

#include <array>
#include <cassert>
#include <expected>
#include <filesystem>
#include <string_view>
#include <vector>

#include "CppReflection/TypeRegistry.hpp"
#include "fmt/core.h"
#include "fmt/std.h"  // IWYU pragma: keep
#include "klgl/error_handling.hpp"
#include "klgl/filesystem/filesystem.hpp"
#include "klgl/reflection/matrix_reflect.hpp"  // IWYU pragma: keep (provides reflection for matrices)
#include "klgl/shader/sampler_uniform.hpp"
#include "klgl/shader/shader.hpp"
#include "klgl/shader/shader_define.hpp"
#include "klgl/shader/shader_uniform.hpp"
#include "klgl/template/on_scope_leave.hpp"
#include "klgl/texture/texture.hpp"
#include "klgl/ui/type_id_widget.hpp"
#include "nlohmann/json.hpp"

namespace klgl
{

std::filesystem::path Shader::shaders_dir_;

struct Shader::Internal
{
    [[nodiscard]] static bool
    TryCompileShader(GlShaderId shader, std::span<const std::string_view> sources, std::string* out_error)
    {
        OpenGl::ShaderSource(shader, sources);
        OpenGl::CompileShader(shader);

        if (OpenGl::GetShaderCompileStatus(shader))
        {
            return true;
        }

        if (out_error)
        {
            if (auto maybe_log = OpenGl::GetShaderLogCE(shader))
            {
                *out_error = std::move(maybe_log.value());
            }
            else
            {
                *out_error = fmt::format("Failed to get shader log. {}", maybe_log.error().message());
            }
        }

        return false;
    }

    [[nodiscard]] static bool
    TryLinkShaderProgram(GlProgramId program, const std::span<const GlShaderId>& shaders, std::string* out_error)
    {
        std::ranges::for_each(shaders, std::bind_front(OpenGl::AttachShader, program));

        OpenGl::LinkProgram(program);
        if (OpenGl::GetProgramLinkStatus(program))
        {
            return true;
        }

        if (out_error)
        {
            [[likely]] if (auto maybe_log = OpenGl::GetProgramLogCE(program))
            {
                *out_error = std::move(maybe_log.value());
            }
            else
            {
                *out_error = fmt::format("Failed to get program log. {}", maybe_log.error().message());
            }
        }

        return false;
    }
};

Shader::Shader(std::filesystem::path path) : path_(std::move(path))
{
    using Clock = std::chrono::high_resolution_clock;
    auto t0 = Clock::now();
    std::string compile_buffer;
    Compile(compile_buffer);
    auto t1 = Clock::now();
    auto d = t1 - t0;
    fmt::println(
        "Compile {} duration: {}",
        path_,
        std::chrono::duration_cast<std::chrono::duration<float, std::milli>>(d));
}

Shader::~Shader()
{
    Destroy();
}

void Shader::Use()
{
    OpenGl::UseProgram(program_);
}

std::optional<uint32_t> Shader::FindUniformLocation(const char* name) const noexcept
{
    return OpenGl::FindUniformLocation(program_.GetValue(), name);
}

uint32_t Shader::GetUniformLocation(const char* name) const noexcept
{
    return OpenGl::GetUniformLocation(program_.GetValue(), name);
}

void Shader::Compile(std::string& buffer)
{
    Destroy();

    // Reuse the buffer to read JSON file
    Filesystem::ReadFile(shaders_dir_ / path_, buffer);
    auto shader_json = nlohmann::json::parse(buffer);
    buffer.clear();

    size_t num_compiled = 0;
    std::array<GlShaderId, 2> shaders{};
    auto deleter = OnScopeLeave(
        [&]()
        {
            for (size_t i = 0; i < num_compiled; ++i)
            {
                OpenGl::DeleteShader(shaders[i]);
            }
        });

    {
        const std::string& version = shader_json.at("glsl_version");
        fmt::format_to(std::back_inserter(buffer), "#version {}\n\n", version);
    }

    if (!definitions_initialized_)
    {
        if (shader_json.contains("definitions"))
        {
            for (const auto& def_json : shader_json["definitions"])
            {
                defines_.push_back(ShaderDefine::ReadFromJson(def_json));
            }
        }

        definitions_initialized_ = true;
    }

    for (const ShaderDefine& def : defines_)
    {
        def.GenDefine(buffer);
    }

    const size_t common_code_length = buffer.size();
    const auto shaders_sources_path = shaders_dir_ / "src";
    auto add_one = [&](GlShaderType type, const std::string_view json_name)
    {
        if (!shader_json.contains(json_name))
        {
            return;
        }

        auto shader = OpenGl::CreateShader(type);
        shaders[num_compiled++] = shader;

        const std::string& src_name = shader_json[json_name];
        Filesystem::AppendFileContentToBuffer(shaders_sources_path / src_name, buffer);

        const std::string_view code_view = buffer;
        std::string compile_log;
        [[unlikely]] if (!Internal::TryCompileShader(shader, std::span{&code_view, 1}, &compile_log))
        {
            throw klgl::ErrorHandling::RuntimeErrorWithMessage(
                "failed to compile shader {} log:\n{}",
                src_name,
                compile_log);
        }

        // remove file content to reuse the code shared across all types of shaders
        buffer.resize(common_code_length);
    };

    add_one(GlShaderType::Vertex, "vertex");
    add_one(GlShaderType::Fragment, "fragment");

    GlProgramId program = OpenGl::CreateProgram();

    auto program_deleter = klgl::OnScopeLeave(
        [&]
        {
            if (program.IsValid())
            {
                OpenGl::DeleteProgram(program);
            }
        });

    std::string link_log;
    [[unlikely]] if (!Internal::TryLinkShaderProgram(program, std::span(shaders).subspan(0, num_compiled), &link_log))
    {
        throw klgl::ErrorHandling::RuntimeErrorWithMessage("Failed to link shader {}. {}", path_, link_log);
    }

    program_ = program;
    program = {};
    need_recompile_ = false;
    UpdateUniforms();
}

void Shader::DrawDetails()
{
    if (ImGui::TreeNode("Static Variables"))
    {
        for (ShaderDefine& definition : defines_)
        {
            bool value_changed = false;
            SimpleTypeWidget(definition.type_guid, definition.name.GetView(), definition.value.data(), value_changed);

            if (value_changed)
            {
                need_recompile_ = true;
            }
        }
        ImGui::TreePop();
    }

    constexpr size_t stack_val_bytes = 64;
    std::array<uint64_t, stack_val_bytes / 8> stack_val_arr{};

    if (ImGui::TreeNode("Dynamic Variables"))
    {
        for (ShaderUniform& uniform : uniforms_)
        {
            auto type_info = cppreflection::GetTypeRegistry()->FindType(uniform.GetTypeGUID());

            type_info->GetSpecialMembers().copyConstructor(
                stack_val_arr.data(),  // NOLINT
                uniform.GetValue().data());
            assert(stack_val_bytes >= type_info->GetInstanceSize());

            std::span<uint8_t> val_view(
                reinterpret_cast<uint8_t*>(stack_val_arr.data()),  // NOLINT
                type_info->GetInstanceSize());

            bool value_changed = false;
            SimpleTypeWidget(uniform.GetTypeGUID(), uniform.GetName().GetView(), val_view.data(), value_changed);

            if (value_changed)
            {
                uniform.SetValue(val_view);
            }

            type_info->GetSpecialMembers().destructor(val_view.data());
        }
        ImGui::TreePop();
    }

    if (need_recompile_)
    {
        std::string buffer;
        Compile(buffer);
    }
}

std::span<const uint8_t> Shader::GetDefineValue(DefineHandle& handle, edt::GUID type_guid) const
{
    UpdateDefineHandle(handle);

    auto& define = defines_[handle.index];
    [[unlikely]] if (define.type_guid != type_guid)
    {
        auto type_registry = cppreflection::GetTypeRegistry();
        throw std::runtime_error(fmt::format(
            "Trying to interpret define {} of type {} as {}",
            define.name.GetView(),
            type_registry->FindType(define.type_guid)->GetName(),
            type_registry->FindType(type_guid)->GetName()));
    }

    return std::span(define.value.begin(), define.value.size());
}

void Shader::SetDefineValue(DefineHandle& handle, edt::GUID type_guid, std::span<const uint8_t> value)
{
    UpdateDefineHandle(handle);
    ShaderDefine& define = defines_[handle.index];
    define.SetValue(value);
    need_recompile_ = true;

    [[unlikely]] if (define.type_guid != type_guid)
    {
        throw std::runtime_error(fmt::format("wrong type"));
    }
}

std::optional<DefineHandle> Shader::FindDefine(Name name) const noexcept
{
    std::optional<DefineHandle> result;
    for (size_t index = 0; index < defines_.size(); ++index)
    {
        const ShaderDefine& define = defines_[index];
        if (define.name == name)
        {
            DefineHandle h;
            h.name = name;
            h.index = static_cast<uint32_t>(index);
            result = h;
            break;
        }
    }

    return result;
}

DefineHandle Shader::GetDefine(Name name) const
{
    [[likely]] if (auto maybe_handle = FindDefine(name); maybe_handle)
    {
        return *maybe_handle;
    }

    throw std::runtime_error(fmt::format("Define is not found: \"{}\"", name.GetView()));
}

std::optional<UniformHandle> Shader::FindUniform(Name name) const noexcept
{
    std::optional<UniformHandle> result;
    for (size_t index = 0; index < uniforms_.size(); ++index)
    {
        const ShaderUniform& uniform = uniforms_[index];
        if (uniform.GetName() == name)
        {
            UniformHandle h;
            h.name = name;
            h.index = static_cast<uint32_t>(index);
            result = h;
            break;
        }
    }

    return result;
}

UniformHandle Shader::GetUniform(Name name) const
{
    [[likely]] if (auto maybe_handle = FindUniform(name); maybe_handle)
    {
        return *maybe_handle;
    }

    fmt::print("Uniform is not found: \"{}\"\n", name.GetView());
    throw std::runtime_error(fmt::format("Uniform is not found: \"{}\"", name.GetView()));
}

ShaderUniform& Shader::GetUniform(UniformHandle& handle)
{
    UpdateUniformHandle(handle);
    return uniforms_[handle.index];
}

const ShaderUniform& Shader::GetUniform(UniformHandle& handle) const
{
    UpdateUniformHandle(handle);
    return uniforms_[handle.index];
}

std::span<const uint8_t> Shader::GetUniformValueViewRaw(UniformHandle& handle, edt::GUID type_guid) const
{
    auto& uniform = GetUniform(handle);
    uniform.EnsureTypeMatch(type_guid);
    return uniform.GetValue();
}

void Shader::UpdateUniformHandle(UniformHandle& handle) const
{
    if (handle.index >= uniforms_.size() || uniforms_[handle.index].GetName() != handle.name)
    {
        handle = GetUniform(handle.name);
    }
}

void Shader::UpdateDefineHandle(DefineHandle& handle) const
{
    [[unlikely]] if (handle.index >= defines_.size() || defines_[handle.index].name != handle.name)
    {
        handle = GetDefine(handle.name);
    }
}

void Shader::SetUniform(UniformHandle& handle, edt::GUID type_guid, std::span<const uint8_t> value)
{
    auto& uniform = GetUniform(handle);
    uniform.EnsureTypeMatch(type_guid);
    uniform.SetValue(value);
}

void Shader::SetUniform(UniformHandle& handle, const Texture& texture)
{
    auto sampler_uniform = GetUniformValue<SamplerUniform>(handle);
    ShaderUniform& u = uniforms_[handle.index];
    sampler_uniform.texture = *texture.GetTexture();
    u.SetValue(std::span(reinterpret_cast<const uint8_t*>(&sampler_uniform), sizeof(sampler_uniform)));  // NOLINT
}

void Shader::SendUniforms()
{
    for (const ShaderUniform& uniform : uniforms_)
    {
        uniform.SendValue();
    }
}

void Shader::SendUniform(UniformHandle& handle)
{
    ShaderUniform& uniform = GetUniform(handle);
    uniform.SendValue();
}

void Shader::Destroy()
{
    if (program_.IsValid())
    {
        OpenGl::DeleteProgram(program_);
        program_ = {};
    }
}

static std::optional<edt::GUID> ConvertGlType(GLenum gl_type)
{
    switch (gl_type)
    {
    case GL_FLOAT:
        return cppreflection::GetStaticTypeInfo<float>().guid;
        break;

    case GL_FLOAT_VEC2:
        return cppreflection::GetStaticTypeInfo<Vec2f>().guid;
        break;

    case GL_FLOAT_VEC3:
        return cppreflection::GetStaticTypeInfo<Vec3f>().guid;
        break;

    case GL_FLOAT_VEC4:
        return cppreflection::GetStaticTypeInfo<Vec4f>().guid;
        break;

    case GL_FLOAT_MAT3:
        return cppreflection::GetStaticTypeInfo<Mat3f>().guid;
        break;

    case GL_FLOAT_MAT4:
        return cppreflection::GetStaticTypeInfo<Mat4f>().guid;
        break;

    case GL_SAMPLER_2D:
        return cppreflection::GetStaticTypeInfo<SamplerUniform>().guid;
        break;
    }

    return std::optional<edt::GUID>();
}

void Shader::UpdateUniforms()
{
    GLuint num_uniforms{};

    {
        GLint num_uniforms_{};
        glGetProgramiv(program_.GetValue(), GL_ACTIVE_UNIFORMS, &num_uniforms_);
        [[unlikely]] if (num_uniforms_ < 1)
        {
            return;
        }
        num_uniforms = static_cast<GLuint>(num_uniforms_);
    }

    GLint max_name_legth = 0;
    glGetProgramiv(program_.GetValue(), GL_ACTIVE_UNIFORM_MAX_LENGTH, &max_name_legth);

    std::string name_buffer_heap;
    constexpr GLsizei name_buffer_size_stack = 64;
    std::array<GLchar, name_buffer_size_stack> name_buffer_stack{};

    GLsizei name_buffer_size{};
    char* name_buffer{};

    if (max_name_legth < name_buffer_size_stack)
    {
        name_buffer = name_buffer_stack.data();
        name_buffer_size = name_buffer_size_stack;
    }
    else
    {
        name_buffer_heap.resize(static_cast<size_t>(max_name_legth));
        name_buffer = name_buffer_heap.data();
        name_buffer_size = static_cast<GLsizei>(max_name_legth);
    }

    std::vector<ShaderUniform> uniforms;
    uniforms.reserve(num_uniforms);
    for (GLuint i = 0; i != num_uniforms; ++i)
    {
        GLint variable_size = 0;
        GLenum glsl_type = 0;
        GLsizei actual_name_length = 0;
        glGetActiveUniform(
            program_.GetValue(),
            i,
            name_buffer_size,
            &actual_name_length,
            &variable_size,
            &glsl_type,
            name_buffer);

        const std::string_view variable_name_view(name_buffer, static_cast<size_t>(actual_name_length));
        const std::optional<edt::GUID> cpp_type = ConvertGlType(glsl_type);
        if (!cpp_type)
        {
            fmt::print("Skip variable {} in \"{}\" - unsupported type", variable_name_view, path_.string());
            continue;
        }

        auto get_or_add = [&uniforms, &cpp_type, this](std::string_view name)
        {
            // find existing variable
            if (auto existing_uniform = std::ranges::find(uniforms_, name, &ShaderUniform::GetNameView);
                existing_uniform != uniforms_.end())
            {
                uniforms.push_back(std::move(*existing_uniform));
                // the previous value can be saved only if variable has the same type
                if (*cpp_type != existing_uniform->GetTypeGUID())
                {
                    uniforms.back().SetType(*cpp_type);
                }
            }
            else
            {
                uniforms.emplace_back();
                auto& uniform = uniforms.back();
                uniform.SetName(Name(name));
                uniform.SetType(*cpp_type);
            }

            const GLint location = glGetUniformLocation(program_.GetValue(), name.data());
            uniforms.back().SetLocation(static_cast<uint32_t>(location));
        };

        if (variable_size == 1)
        {
            get_or_add(variable_name_view);
        }
        else
        {
            std::string name_with_index(variable_name_view);
            const size_t name_no_index_size = name_with_index.find('[');
            for (size_t element_index = 0; element_index != static_cast<size_t>(variable_size); ++element_index)
            {
                name_with_index.resize(name_no_index_size);
                fmt::format_to(std::back_inserter(name_with_index), "[{}]", element_index);
                get_or_add(name_with_index);
            }
        }
    }

    std::swap(uniforms, uniforms_);

    for (size_t i = 0; i < uniforms_.size(); ++i)
    {
        ShaderUniform& uniform = uniforms_[i];
        constexpr edt::GUID sampler_uniform_guid = cppreflection::GetStaticTypeInfo<SamplerUniform>().guid;
        if (uniform.GetTypeGUID() == sampler_uniform_guid)
        {
            UniformHandle handle(static_cast<uint32_t>(i), uniform.GetName());
            auto sampler = GetUniformValue<SamplerUniform>(handle);
            sampler.sampler_index = static_cast<uint8_t>(i);
            uniform.SetValue(std::span(reinterpret_cast<const uint8_t*>(&sampler), sizeof(sampler)));  // NOLINT
        }
    }
}

}  // namespace klgl
