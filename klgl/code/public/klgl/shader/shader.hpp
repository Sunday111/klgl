#pragma once

#include <filesystem>
#include <optional>
#include <span>
#include <vector>

#include "CppReflection/GetStaticTypeInfo.hpp"
#include "klgl/opengl/identifiers.hpp"
#include "klgl/opengl/object.hpp"
#include "klgl/opengl/program_info.hpp"
#include "klgl/shader/define_handle.hpp"
#include "klgl/shader/uniform_handle.hpp"

namespace klgl
{

class ShaderDefine;
class ShaderUniform;
class Texture;

class Shader
{
    struct Internal;

public:
    explicit Shader(std::filesystem::path path);
    ~Shader();

    void Use();

    [[nodiscard]] const GlProgramInfo& GetInfo() const { return info_; }

    void Compile(std::string& buffer);
    void Compile()
    {
        std::string buffer;
        Compile(buffer);
    }
    [[nodiscard]] std::optional<uint32_t> FindUniformLocation(const char*) const noexcept;
    [[nodiscard]] uint32_t GetUniformLocation(const char*) const;
    void DrawDetails();

    std::optional<UniformHandle> FindUniform(Name name) const noexcept;
    UniformHandle GetUniform(Name name) const;
    void SetUniform(UniformHandle& handle, edt::GUID type_guid, std::span<const uint8_t> data);

    void SetUniform(UniformHandle& handle, const Texture& texture);

    template <typename T>
    void SetUniform(UniformHandle& handle, const T& value)
    {
        SetUniform(
            handle,
            cppreflection::GetStaticTypeInfo<T>().guid,
            std::span<const uint8_t>(reinterpret_cast<const uint8_t*>(&value), sizeof(T)));  // NOLINT
    }

    void SendUniforms();
    void SendUniform(UniformHandle&);

    template <typename T>
    const T& GetDefineValue(DefineHandle& handle) const;
    std::span<const uint8_t> GetDefineValue(DefineHandle& handle, edt::GUID type_guid) const;
    std::optional<DefineHandle> FindDefine(Name name) const noexcept;
    DefineHandle GetDefine(Name name) const;

    void SetDefineValue(DefineHandle& handle, edt::GUID type_guid, std::span<const uint8_t> value);

    template <typename T>
    void SetDefineValue(DefineHandle& handle, const T& value)
    {
        SetDefineValue(
            handle,
            cppreflection::GetStaticTypeInfo<T>().guid,
            std::span<const uint8_t>(reinterpret_cast<const uint8_t*>(&value), sizeof(T)));  // NOLINT
    }

    GlProgramId GetProgramId() const { return program_.GetId(); }

protected:
    ShaderUniform& GetUniform(UniformHandle& handle);
    const ShaderUniform& GetUniform(UniformHandle& handle) const;
    std::span<const uint8_t> GetUniformValueViewRaw(UniformHandle& handle, edt::GUID type_guid) const;
    void UpdateUniformHandle(UniformHandle& handle) const;
    void UpdateDefineHandle(DefineHandle& handle) const;

    template <typename T>
    const T& GetUniformValue(UniformHandle& handle)
    {
        std::span<const uint8_t> view = GetUniformValueViewRaw(handle, cppreflection::GetStaticTypeInfo<T>().guid);
        assert(view.size() == sizeof(T));
        return *reinterpret_cast<const T*>(view.data());  // NOLINT
    }

private:
    void UpdateInfo();
    void UpdateUniforms();

public:
    static std::filesystem::path shaders_dir_;

private:
    std::filesystem::path path_;
    std::vector<ShaderDefine> defines_;
    std::vector<ShaderUniform> uniforms_;
    GlProgramInfo info_;
    GlObject<GlProgramId> program_;
    bool definitions_initialized_ : 1 = false;
    bool need_recompile_ : 1 = true;
};

template <typename T>
const T& Shader::GetDefineValue(DefineHandle& handle) const
{
    constexpr edt::GUID type_guid = cppreflection::GetStaticTypeInfo<T>().guid;
    auto value_view = GetDefineValue(handle, type_guid);
    return *reinterpret_cast<const T*>(value_view.data());  // NOLINT
}

}  // namespace klgl
