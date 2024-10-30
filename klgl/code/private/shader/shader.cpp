#include "klgl/shader/shader.hpp"

#include <fmt/chrono.h>
#include <fmt/std.h>
#include <imgui.h>

#include <array>
#include <cassert>
#include <filesystem>
#include <string_view>
#include <tl/expected.hpp>
#include <vector>

#include "CppReflection/TypeRegistry.hpp"
#include "fmt/core.h"
#include "fmt/std.h"  // IWYU pragma: keep
#include "klgl/error_handling.hpp"
#include "klgl/filesystem/filesystem.hpp"
#include "klgl/opengl/detail/maps/to_gl_value/uniform_type.hpp"
#include "klgl/opengl/gl_api.hpp"
#include "klgl/opengl/program_info.hpp"
#include "klgl/reflection/matrix_reflect.hpp"  // IWYU pragma: keep (provides reflection for matrices)
#include "klgl/shader/sampler_uniform.hpp"
#include "klgl/shader/shader.hpp"
#include "klgl/shader/shader_define.hpp"
#include "klgl/shader/shader_uniform.hpp"
#include "klgl/template/constexpr_string_hash.hpp"
#include "klgl/texture/texture.hpp"
#include "klgl/ui/imgui_helpers.hpp"
#include "klgl/ui/type_id_widget_minimal.hpp"
#include "nlohmann/json.hpp"

namespace klgl
{

std::filesystem::path Shader::shaders_dir_;

struct Shader::Internal
{
    [[nodiscard]] static bool TryCompileShader(
        const GlObject<GlShaderId>& shader,
        std::span<const std::string_view> sources,
        std::string* out_error)
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

    [[nodiscard]] static bool TryLinkShaderProgram(
        const GlObject<GlProgramId>& program,
        const std::span<const GlObject<GlShaderId>>& shaders,
        std::string* out_error)
    {
        for (auto& shader : shaders)
        {
            OpenGl::AttachShader(program, shader);
        }

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
    std::string compile_buffer;
    Compile(compile_buffer);
}

Shader::~Shader() = default;

void Shader::Use()
{
    OpenGl::UseProgram(program_);
}

std::optional<uint32_t> Shader::FindUniformLocation(const char* name) const noexcept
{
    if (GLint location = OpenGl::GetUniformLocation(program_, name); location >= 0)
    {
        return static_cast<uint32_t>(location);
    }

    return std::nullopt;
}

uint32_t Shader::GetUniformLocation(const char* name) const
{
    if (auto maybe_location = FindUniformLocation(name))
    {
        return maybe_location.value();
    }

    throw ErrorHandling::RuntimeErrorWithMessage(
        "Could not find location of uniform with name \"{}\" in shader \"{}\"",
        name,
        path_.stem());
}

static constexpr auto kExtensionToShaderType = []()
{
    constexpr size_t num_shader_types = magic_enum::enum_count<GlShaderType>();
    ass::FixedUnorderedMap<num_shader_types, std::string_view, GlShaderType, ConstexprStringHasher> m;
    m.Add(".vert", GlShaderType::Vertex);
    m.Add(".geom", GlShaderType::Geometry);
    m.Add(".frag", GlShaderType::Fragment);
    m.Add(".tesc", GlShaderType::TesselationControl);
    m.Add(".tese", GlShaderType::TesselationEvaluation);
    m.Add(".comp", GlShaderType::Compute);
    return m;
}();

void Shader::Compile(std::string& buffer)
{
    program_ = {};

    auto shader_dir = shaders_dir_ / path_;

    std::optional<std::filesystem::path> json_file_path;
    ass::EnumMap<GlShaderType, std::filesystem::path> type_to_path;
    for (const auto& entry : std::filesystem::directory_iterator(shader_dir))
    {
        if (entry.is_directory()) continue;

        const auto& path = entry.path();
        const std::string ext = path.extension().string();

        if (ext == ".json")
        {
            [[unlikely]] if (json_file_path.has_value())
            {
                ErrorHandling::ThrowWithMessage(
                    "There are at least two {} files in {}: \"{}\" and \"{}\". At most one is allowed.",
                    ext,
                    shader_dir,
                    std::filesystem::relative(json_file_path.value(), shader_dir),
                    std::filesystem::relative(path, shader_dir));
            }
            json_file_path = path;
        }

        if (!kExtensionToShaderType.Contains(ext)) continue;

        const GlShaderType type = kExtensionToShaderType.Get(ext);

        [[unlikely]] if (type_to_path.Contains(type))
        {
            ErrorHandling::ThrowWithMessage(
                "There are at least two files with same extension {} in {}: {} and {}",
                ext,
                shader_dir,
                type_to_path.Get(type),
                path);
        }

        type_to_path.Emplace(type, path);
    }

    // Reuse the buffer to read JSON file
    std::optional<nlohmann::json> maybe_config;
    if (json_file_path)
    {
        Filesystem::ReadFile(json_file_path.value(), buffer);
        maybe_config = nlohmann::json::parse(buffer);
        buffer.clear();
    }

    size_t num_compiled = 0;
    std::array<GlObject<GlShaderId>, 3> shaders{};

    {
        std::string_view version = "330 core";
        if (maybe_config && maybe_config->contains("glsl_version"))
        {
            const auto& value = maybe_config->at("glsl_version");
            const auto& version_str = value.get_ref<const nlohmann::json::string_t&>();
            version = version_str;
        }
        fmt::format_to(std::back_inserter(buffer), "#version {}\n\n", version);
    }

    if (!definitions_initialized_)
    {
        if (maybe_config && maybe_config->contains("definitions"))
        {
            const auto& config = *maybe_config;
            for (const auto& def_json : config["definitions"])
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

    // Resets the line number to display correct line number in logs if there is an error
    fmt::format_to(std::back_inserter(buffer), "#line 1\n");

    const size_t common_code_length = buffer.size();
    auto add_one = [&](GlShaderType type)
    {
        if (!type_to_path.Contains(type)) return;

        shaders[num_compiled] = GlObject<GlShaderId>::CreateFrom(OpenGl::CreateShader(type));
        const auto& shader = shaders[num_compiled];
        num_compiled++;

        const auto& path = type_to_path.Get(type);
        Filesystem::AppendFileContentToBuffer(path, buffer);

        const std::string_view code_view = buffer;
        std::string compile_log;
        [[unlikely]] if (!Internal::TryCompileShader(shader, std::span{&code_view, 1}, &compile_log))
        {
            throw klgl::ErrorHandling::RuntimeErrorWithMessage(
                "failed to compile shader {} log:\n{}",
                path,
                compile_log);
        }

        // remove file content to reuse the code shared across all types of shaders
        buffer.resize(common_code_length);
    };

    add_one(GlShaderType::Vertex);
    add_one(GlShaderType::Geometry);
    add_one(GlShaderType::Fragment);

    auto program = GlObject<GlProgramId>::CreateFrom(OpenGl::CreateProgram());

    std::string link_log;
    [[unlikely]] if (!Internal::TryLinkShaderProgram(program, std::span(shaders).subspan(0, num_compiled), &link_log))
    {
        throw klgl::ErrorHandling::RuntimeErrorWithMessage("Failed to link shader {}. {}", path_, link_log);
    }

    program_ = std::move(program);
    need_recompile_ = false;
    UpdateInfo();
    UpdateUniforms();
}

void Shader::DrawDetails()
{
    std::string buffer;

    if (ImGui::TreeNode("Relection"))
    {
        if (!info_.vertex_attributes.empty() && ImGui::TreeNode("Vertex attributes"))
        {
            for (const auto& vertex_attribute : info_.vertex_attributes)
            {
                if (ImGui::TreeNode(vertex_attribute.name.data()))
                {
                    ImGuiHelper::FormattedText(buffer, "Index: {}", vertex_attribute.index);
                    ImGuiHelper::FormattedText(buffer, "Type: {}", vertex_attribute.type);
                    ImGuiHelper::FormattedText(buffer, "Location: {}", vertex_attribute.location);
                    ImGuiHelper::FormattedText(buffer, "Size: {}", vertex_attribute.size);
                    ImGui::TreePop();
                }
            }

            ImGui::TreePop();
        }

        if (!info_.uniforms.empty() && ImGui::TreeNode("Uniforms"))
        {
            for (const auto& uniform : info_.uniforms)
            {
                if (ImGui::TreeNode(uniform.name.data()))
                {
                    ImGuiHelper::FormattedText(buffer, "Index: {}", uniform.index);
                    ImGuiHelper::FormattedText(buffer, "Type: {}", uniform.type);
                    ImGuiHelper::FormattedText(buffer, "Location: {}", uniform.location);
                    ImGuiHelper::FormattedText(buffer, "Size: {}", uniform.size);
                    ImGui::TreePop();
                }
            }

            ImGui::TreePop();
        }

        ImGui::TreePop();
    }

    if (ImGui::TreeNode("Static Variables"))
    {
        for (ShaderDefine& definition : defines_)
        {
            const bool value_changed =
                SimpleTypeWidget(definition.type_guid, definition.name.GetView(), definition.value.data());

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

            const bool value_changed =
                SimpleTypeWidget(uniform.GetTypeGUID(), uniform.GetName().GetView(), val_view.data());

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
    sampler_uniform.texture = texture.GetTexture();
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

static std::optional<edt::GUID> ConvertGlType(GLenum gl_type)
{
    switch (gl_type)
    {
    case GL_INT:
        return cppreflection::GetStaticTypeInfo<int32_t>().guid;
        break;

    case GL_UNSIGNED_INT:
        return cppreflection::GetStaticTypeInfo<uint32_t>().guid;
        break;

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

void Shader::UpdateInfo()
{
    info_.FetchUniforms(program_);
    info_.FetchVertexAttributes(program_);
}

void Shader::UpdateUniforms()
{
    std::vector<ShaderUniform> uniforms;
    uniforms.reserve(info_.uniforms.size());
    for (size_t i = 0; i != info_.uniforms.size(); ++i)
    {
        const auto& uniform_info = info_.uniforms[i];

        const std::optional<edt::GUID> cpp_type = ConvertGlType(ToGlValue(uniform_info.type));
        if (!cpp_type)
        {
            fmt::print("Skip variable {} in \"{}\" - unsupported type", uniform_info.name, path_.string());
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

            const GLint location = glGetUniformLocation(program_.GetId().GetValue(), name.data());
            uniforms.back().SetLocation(static_cast<uint32_t>(location));
        };

        if (uniform_info.size == 1)
        {
            get_or_add(uniform_info.name);
        }
        else
        {
            std::string name_with_index(uniform_info.name);
            const size_t name_no_index_size = name_with_index.find('[');
            for (size_t element_index = 0; element_index != uniform_info.size; ++element_index)
            {
                name_with_index.resize(name_no_index_size);
                fmt::format_to(std::back_inserter(name_with_index), "[{}]", element_index);
                get_or_add(name_with_index);
            }
        }
    }

    std::swap(uniforms, uniforms_);

    // Update index for sampler uniforms
    for (size_t i = 0; i < uniforms_.size(); ++i)
    {
        ShaderUniform& uniform = uniforms_[i];
        if (uniform.GetTypeGUID() == cppreflection::GetStaticTypeInfo<SamplerUniform>().guid)
        {
            UniformHandle handle(static_cast<uint32_t>(i), uniform.GetName());
            auto sampler = GetUniformValue<SamplerUniform>(handle);
            sampler.sampler_index = static_cast<uint8_t>(i);
            uniform.SetValue(std::span(reinterpret_cast<const uint8_t*>(&sampler), sizeof(sampler)));  // NOLINT
        }
    }
}

}  // namespace klgl
