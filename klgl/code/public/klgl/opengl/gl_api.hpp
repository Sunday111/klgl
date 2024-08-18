#pragma once

#include <glad/glad.h>

#include <optional>
#include <span>
#include <string_view>

#include "CppReflection/GetStaticTypeInfo.hpp"
#include "EverydayTools/Math/Matrix.hpp"
#include "enums.hpp"

namespace klgl
{

using namespace edt::lazy_matrix_aliases;  // NOLINT

class OpenGl
{
public:
    [[nodiscard]] static GLuint GenVertexArray() noexcept;
    static void GenVertexArrays(const std::span<GLuint>& arrays) noexcept;

    static void BindVertexArray(GLuint array) noexcept;

    [[nodiscard]] static GLuint GenBuffer() noexcept;
    static void GenBuffers(const std::span<GLuint>& buffers) noexcept;

    [[nodiscard]] static GLuint GenTexture() noexcept;
    static void GenTextures(const std::span<GLuint>& textures) noexcept;

    static void BindBuffer(GLenum target, GLuint buffer) noexcept;

    static void BufferData(GLenum target, GLsizeiptr size, const void* data, GLenum usage) noexcept;

    template <typename T, size_t Extent>
    static void BufferData(GLenum target, const std::span<T, Extent>& data, GLenum usage) noexcept
    {
        BufferData(target, static_cast<GLsizeiptr>(sizeof(T) * data.size()), data.data(), usage);
    }

    template <typename T, size_t Extent>
    static void BufferData(GLenum target, const std::span<const T, Extent>& data, GLenum usage) noexcept
    {
        BufferData(target, static_cast<GLsizeiptr>(sizeof(T) * data.size()), data.data(), usage);
    }

    [[nodiscard]] static constexpr GLboolean CastBool(bool value) noexcept;

    static void VertexAttribPointer(
        GLuint index,
        size_t size,
        GLenum type,
        bool normalized,
        size_t stride,
        const void* pointer) noexcept;

    static void EnableVertexAttribArray(GLuint index) noexcept;
    static void EnableDepthTest() noexcept;
    static void EnableBlending() noexcept;

    static void Viewport(GLint x, GLint y, GLsizei width, GLsizei height) noexcept;

    static void SetClearColor(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha) noexcept;
    static void SetClearColor(const Vec4f& color) noexcept;

    static void Clear(GLbitfield mask) noexcept;

    static void UseProgram(GLuint program) noexcept;

    static void DrawElements(GLenum mode, size_t num, GLenum indices_type, const void* indices) noexcept;

    static void SetUniform(uint32_t location, const float& f) noexcept;

    static void SetUniform(uint32_t location, const Mat4f& m, bool transpose = false) noexcept;

    static void SetUniform(uint32_t location, const Mat3f& m, bool transpose = false) noexcept;

    static void SetUniform(uint32_t location, const Vec4f& v) noexcept;

    static void SetUniform(uint32_t location, const Vec3f& v) noexcept;

    static void SetUniform(uint32_t location, const Vec2f& v) noexcept;

    static void SetTextureParameter(GLenum target, GLenum pname, const GLfloat* value) noexcept;

    static void SetTextureParameter(GLenum target, GLenum name, GLint param) noexcept;

    template <typename T>
    static void SetTextureParameter2d(GLenum pname, T value) noexcept
    {
        SetTextureParameter(GL_TEXTURE_2D, pname, value);
    }

    static void SetTexture2dBorderColor(const Vec4f& v) noexcept;

    static void SetTexture2dWrap(GlTextureWrap wrap, GlTextureWrapMode mode) noexcept;

    static void SetTexture2dMinFilter(GlTextureFilter filter) noexcept;

    static void SetTexture2dMagFilter(GlTextureFilter filter) noexcept;

    static void BindTexture(GLenum target, GLuint texture) noexcept;

    static void BindTexture2d(GLuint texture);

    static void TexImage2d(
        GLenum target,
        size_t level_of_detail,
        GLint internal_format,
        size_t width,
        size_t height,
        GLint data_format,
        GLenum pixel_data_type,
        const void* pixels) noexcept;

    static void GenerateMipmap(GLenum target) noexcept;

    static void GenerateMipmap2d() noexcept;

    [[nodiscard]] static std::optional<uint32_t> FindUniformLocation(GLuint shader_program, const char* name) noexcept;

    [[nodiscard]] static uint32_t GetUniformLocation(GLuint shader_program, const char* name);

    static void PolygonMode(GlPolygonMode mode) noexcept;
    static void PointSize(float size) noexcept;
    static void LineWidth(float width) noexcept;
};

}  // namespace klgl

namespace cppreflection
{
template <>
struct TypeReflectionProvider<::klgl::GlPolygonMode>
{
    [[nodiscard]] inline constexpr static auto ReflectType()
    {
        return cppreflection::StaticEnumTypeInfo<::klgl::GlPolygonMode>(
                   "GlPolygonMode",
                   edt::GUID::Create("B6808C23-A1BA-42A5-AA31-F08ED15D3AC9"))
            .Value(::klgl::GlPolygonMode::Point, "Point")
            .Value(::klgl::GlPolygonMode::Line, "Line")
            .Value(::klgl::GlPolygonMode::Fill, "Fill")
            .Value(::klgl::GlPolygonMode::kMax, "Max");
    }
};

template <>
struct TypeReflectionProvider<::klgl::GlTextureWrap>
{
    [[nodiscard]] inline constexpr static auto ReflectType()
    {
        return cppreflection::StaticEnumTypeInfo<::klgl::GlTextureWrap>(
                   "GlTextureWrap",
                   edt::GUID::Create("8D676F4B-F1B5-4F80-8772-125376832D8E"))
            .Value(::klgl::GlTextureWrap::S, "S")
            .Value(::klgl::GlTextureWrap::T, "T")
            .Value(::klgl::GlTextureWrap::R, "R")
            .Value(::klgl::GlTextureWrap::kMax, "Max");
    }
};

template <>
struct TypeReflectionProvider<::klgl::GlTextureWrapMode>
{
    [[nodiscard]] inline constexpr static auto ReflectType()
    {
        return cppreflection::StaticEnumTypeInfo<::klgl::GlTextureWrapMode>(
                   "GlTextureWrapMode",
                   edt::GUID::Create("668C28DB-01FD-47DE-A3B9-7081A1B68CC4"))
            .Value(::klgl::GlTextureWrapMode::ClampToEdge, "ClampToEdge")
            .Value(::klgl::GlTextureWrapMode::ClampToBorder, "ClampToBorder")
            .Value(::klgl::GlTextureWrapMode::MirroredRepeat, "MirroredRepeat")
            .Value(::klgl::GlTextureWrapMode::Repeat, "Repeat")
            .Value(::klgl::GlTextureWrapMode::MirrorClampToEdge, "MirrorClampToEdge")
            .Value(::klgl::GlTextureWrapMode::kMax, "Max");
    }
};

template <>
struct TypeReflectionProvider<::klgl::GlTextureFilter>
{
    [[nodiscard]] inline constexpr static auto ReflectType()
    {
        return cppreflection::StaticEnumTypeInfo<::klgl::GlTextureFilter>(
                   "GlTextureFilter",
                   edt::GUID::Create("73D29DE8-C8B0-4C97-897B-1154C0D0ABBB"))
            .Value(::klgl::GlTextureFilter::Nearest, "Nearest")
            .Value(::klgl::GlTextureFilter::Linear, "Linear")
            .Value(::klgl::GlTextureFilter::NearestMipmapNearest, "NearestMipmapNearest")
            .Value(::klgl::GlTextureFilter::LinearMipmapNearest, "LinearMipmapNearest")
            .Value(::klgl::GlTextureFilter::NearestMipmapLinear, "NearestMipmapLinear")
            .Value(::klgl::GlTextureFilter::LinearMipmapLinear, "LinearMipmapLinear")
            .Value(::klgl::GlTextureFilter::kMax, "Max");
    }
};
}  // namespace cppreflection
