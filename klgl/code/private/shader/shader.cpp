#include "klgl/shader/shader.hpp"

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

static size_t GetShaderLogLength(GLuint shader)
{
    GLint log_length{};
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_length);
    return static_cast<size_t>(std::max(0, log_length));
}

static std::string GetShaderLog(GLuint shader)
{
    std::string log;
    const size_t length = GetShaderLogLength(shader);
    if (length != 0)
    {
        log.resize(length);
        glGetShaderInfoLog(shader, static_cast<GLint>(length), nullptr, log.data());
    }

    return log;
}

static size_t GetProgramLogLength(GLuint program)
{
    GLint log_length{};
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &log_length);
    return static_cast<size_t>(std::max(0, log_length));
}

static std::string GetProgramLog(GLuint program)
{
    std::string log;
    const size_t length = GetProgramLogLength(program);
    if (length != 0)
    {
        log.resize(length);
        glGetProgramInfoLog(program, static_cast<GLint>(length), nullptr, log.data());
    }

    return log;
}

[[nodiscard]] static std::optional<GLuint>
TryCompileShader(const GLuint type, std::span<const std::string_view> sources, std::string* out_error)
{
    constexpr size_t stack_reserved = 30;

    std::vector<const char*> shader_sources_heap;
    std::vector<GLint> shader_sources_lengths_heap;
    std::array<const char*, stack_reserved> shader_sources_stack{};
    std::array<GLint, stack_reserved> shader_sources_lengths_stack{};
    const size_t num_sources = sources.size();
    std::span<const char*> shader_sources;
    std::span<GLint> shader_sources_lengths{};
    if (num_sources > shader_sources_stack.size())
    {
        shader_sources_heap.resize(num_sources);
        shader_sources_lengths_heap.resize(num_sources);
        shader_sources = shader_sources_heap;
        shader_sources_lengths = shader_sources_lengths_heap;
    }
    else
    {
        shader_sources = shader_sources_stack;
        shader_sources_lengths = shader_sources_lengths_stack;
    }

    for (size_t i = 0; i < sources.size(); ++i)
    {
        shader_sources[i] = sources[i].data();
        shader_sources_lengths[i] = static_cast<GLsizei>(sources[i].size());
    }

    const GLuint shader = glCreateShader(type);
    glShaderSource(shader, static_cast<GLsizei>(num_sources), shader_sources.data(), shader_sources_lengths.data());
    glCompileShader(shader);

    int success{};
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

    [[likely]] if (success)
    {
        return shader;
    }

    auto deleter = klgl::OnScopeLeave([&] { glDeleteShader(shader); });

    if (out_error)
    {
        *out_error = GetShaderLog(shader);
    }

    return std::nullopt;
}

[[nodiscard]] static std::expected<GLuint, std::string> TryCompileShader(
    const GLuint type,
    std::span<const std::string_view> sources)
{
    std::string error;
    [[likely]] if (auto opt_program = TryCompileShader(type, sources, &error))
    {
        return opt_program.value();
    }

    return std::unexpected{std::move(error)};
}

[[nodiscard]] static std::optional<GLuint> TryLinkShaderProgram(
    const std::span<const GLuint>& shaders,
    std::string* out_error)
{
    GLuint program = glCreateProgram();
    for (auto shader : shaders)
    {
        glAttachShader(program, shader);
    }

    glLinkProgram(program);
    int success{};
    glGetProgramiv(program, GL_LINK_STATUS, &success);

    [[likely]] if (success)
    {
        return program;
    }

    auto deleter = klgl::OnScopeLeave([&] { glDeleteProgram(program); });

    if (out_error)
    {
        *out_error = GetProgramLog(program);
    }

    return std::nullopt;
}

[[nodiscard]] static std::expected<GLuint, std::string> TryLinkShaderProgram(const std::span<const GLuint>& shaders)
{
    std::string error;
    [[likely]] if (auto opt_program = TryLinkShaderProgram(shaders, &error))
    {
        return opt_program.value();
    }

    return std::unexpected{std::move(error)};
}

Shader::Shader(std::filesystem::path path) : path_(std::move(path))
{
    std::string compile_buffer;
    Compile(compile_buffer);
}

Shader::~Shader()
{
    Destroy();
}
void Shader::Use()
{
    Check();
    OpenGl::UseProgram(*program_);
}

std::optional<uint32_t> Shader::FindUniformLocation(const char* name) const noexcept
{
    Check();
    return OpenGl::FindUniformLocation(*program_, name);
}

uint32_t Shader::GetUniformLocation(const char* name) const noexcept
{
    Check();
    return OpenGl::GetUniformLocation(*program_, name);
}

void Shader::Compile(std::string& buffer)
{
    Destroy();

    // Reuse the buffer to read JSON file
    Filesystem::ReadFile(shaders_dir_ / path_, buffer);
    auto shader_json = nlohmann::json::parse(buffer);
    buffer.clear();

    size_t num_compiled = 0;
    std::array<GLuint, 2> compiled{};
    auto deleter = OnScopeLeave(
        [&]()
        {
            for (size_t i = 0; i < num_compiled; ++i)
            {
                glDeleteShader(compiled[i]);
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
    auto add_one = [&](GLuint type, const std::string_view json_name)
    {
        if (!shader_json.contains(json_name))
        {
            return;
        }

        const std::string& src_name = shader_json[json_name];
        Filesystem::AppendFileContentToBuffer(shaders_sources_path / src_name, buffer);

        const std::string_view code_view = buffer;
        const auto compile_result = TryCompileShader(type, std::span{&code_view, 1});

        // remove file content to reuse the code shared across all types of shaders
        buffer.resize(common_code_length);

        [[likely]] if (compile_result.has_value())
        {
            compiled[num_compiled] = compile_result.value();
            num_compiled += 1;
        }
        else
        {
            throw klgl::ErrorHandling::RuntimeErrorWithMessage(
                "failed to compile shader {} log:\n{}",
                src_name,
                compile_result.error());
        }
    };

    add_one(GL_VERTEX_SHADER, "vertex");
    add_one(GL_FRAGMENT_SHADER, "fragment");

    [[likely]] if (auto link_result = TryLinkShaderProgram(std::span(compiled).subspan(0, num_compiled)))
    {
        program_ = link_result.value();
        need_recompile_ = false;
        UpdateUniforms();
    }
    else
    {
        throw klgl::ErrorHandling::RuntimeErrorWithMessage("Failed to link shader {}. {}", path_, link_result.error());
    }
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

void Shader::Check() const
{
    [[unlikely]] if (!program_)
    {
        throw std::runtime_error("Invalid shader");
    }
}

void Shader::Destroy()
{
    if (program_)
    {
        glDeleteProgram(*program_);
        program_.reset();
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
        glGetProgramiv(*program_, GL_ACTIVE_UNIFORMS, &num_uniforms_);
        [[unlikely]] if (num_uniforms_ < 1)
        {
            return;
        }
        num_uniforms = static_cast<GLuint>(num_uniforms_);
    }

    GLint max_name_legth = 0;
    glGetProgramiv(*program_, GL_ACTIVE_UNIFORM_MAX_LENGTH, &max_name_legth);

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
            *program_,
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

            const GLint location = glGetUniformLocation(*program_, name.data());
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
            UniformHandle handle{static_cast<uint32_t>(i), uniform.GetName()};
            auto sampler = GetUniformValue<SamplerUniform>(handle);
            sampler.sampler_index = static_cast<uint8_t>(i);
            uniform.SetValue(std::span(reinterpret_cast<const uint8_t*>(&sampler), sizeof(sampler)));  // NOLINT
        }
    }
}

}  // namespace klgl
