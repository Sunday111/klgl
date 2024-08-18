#pragma once

#include <glad/glad.h>

#include <optional>
#include <span>

#include "EverydayTools/Math/Matrix.hpp"
#include "enums.hpp"

namespace klgl
{

using namespace edt::lazy_matrix_aliases;  // NOLINT

class OpenGl
{
public:
    struct Internal;

    [[nodiscard]] static GlError GetError() noexcept;

    [[nodiscard]] static GLuint GenBuffer() noexcept;
    static void GenBuffers(const std::span<GLuint>& buffers) noexcept;

    [[nodiscard]] static GLuint GenVertexArray() noexcept;
    static void GenVertexArrays(const std::span<GLuint>& arrays) noexcept;

    [[nodiscard]] static GLuint GenTexture() noexcept;
    static void GenTextures(const std::span<GLuint>& textures) noexcept;

    static void BindBufferNE(GlBufferType target, GLuint buffer) noexcept;
    static void BindBuffer(GlBufferType target, GLuint buffer);

    static void BindVertexArrayNE(GLuint array) noexcept;
    static void BindVertexArray(GLuint array);

    static void BufferDataNE(GlBufferType target, GLsizeiptr size, const void* data, GlUsage usage) noexcept;
    static void BufferData(GlBufferType target, GLsizeiptr size, const void* data, GlUsage usage);

    template <typename T, size_t Extent>
    static void BufferDataNE(GlBufferType target, const std::span<T, Extent>& data, GlUsage usage) noexcept
    {
        BufferDataNE(target, static_cast<GLsizeiptr>(sizeof(T) * data.size()), data.data(), usage);
    }

    template <typename T, size_t Extent>
    static void BufferData(GlBufferType target, const std::span<T, Extent>& data, GlUsage usage)
    {
        BufferData(target, static_cast<GLsizeiptr>(sizeof(T) * data.size()), data.data(), usage);
    }

    template <typename T, size_t Extent>
    static void BufferDataNE(GlBufferType target, const std::span<const T, Extent>& data, GlUsage usage) noexcept
    {
        BufferDataNE(target, static_cast<GLsizeiptr>(sizeof(T) * data.size()), data.data(), usage);
    }

    template <typename T, size_t Extent>
    static void BufferData(GlBufferType target, const std::span<const T, Extent>& data, GlUsage usage) noexcept
    {
        BufferData(target, static_cast<GLsizeiptr>(sizeof(T) * data.size()), data.data(), usage);
    }

    [[nodiscard]] static constexpr GLboolean CastBool(bool value) noexcept;

    static void VertexAttribPointerNE(
        GLuint index,
        size_t size,
        GLenum type,
        bool normalized,
        size_t stride,
        const void* pointer) noexcept;
    static void
    VertexAttribPointer(GLuint index, size_t size, GLenum type, bool normalized, size_t stride, const void* pointer);

    static void EnableVertexAttribArrayNE(GLuint index) noexcept;
    static void EnableVertexAttribArray(GLuint index);

    static void EnableDepthTestNE() noexcept;
    static void EnableDepthTest();

    static void EnableBlendingNE() noexcept;
    static void EnableBlending();

    static void ViewportNE(GLint x, GLint y, GLsizei width, GLsizei height) noexcept;
    static void Viewport(GLint x, GLint y, GLsizei width, GLsizei height);

    static void SetClearColorNE(GLfloat r, GLfloat g, GLfloat b, GLfloat a) noexcept;
    static void SetClearColor(GLfloat r, GLfloat g, GLfloat b, GLfloat a);

    static void SetClearColorNE(const Vec4f& color) noexcept;
    static void SetClearColor(const Vec4f& color);

    static void ClearNE(GLbitfield mask) noexcept;
    static void Clear(GLbitfield mask) noexcept;

    static void UseProgramNE(GLuint program) noexcept;
    static void UseProgram(GLuint program);

    static void DrawElementsNE(GLenum mode, size_t num, GLenum indices_type, const void* indices) noexcept;
    static void DrawElements(GLenum mode, size_t num, GLenum indices_type, const void* indices);

    static void SetUniformNE(uint32_t location, const float& f) noexcept;
    static void SetUniform(uint32_t location, const float& f);

    static void SetUniformNE(uint32_t location, const Mat4f& m, bool transpose = false) noexcept;
    static void SetUniform(uint32_t location, const Mat4f& m, bool transpose = false);

    static void SetUniformNE(uint32_t location, const Mat3f& m, bool transpose = false) noexcept;
    static void SetUniform(uint32_t location, const Mat3f& m, bool transpose = false);

    static void SetUniformNE(uint32_t location, const Vec4f& v) noexcept;
    static void SetUniform(uint32_t location, const Vec4f& v);

    static void SetUniformNE(uint32_t location, const Vec3f& v) noexcept;
    static void SetUniform(uint32_t location, const Vec3f& v);

    static void SetUniformNE(uint32_t location, const Vec2f& v) noexcept;
    static void SetUniform(uint32_t location, const Vec2f& v);

    static void SetTextureParameterNE(GLenum target, GLenum pname, const GLfloat* value) noexcept;
    static void SetTextureParameter(GLenum target, GLenum pname, const GLfloat* value);

    static void SetTextureParameterNE(GLenum target, GLenum name, GLint param) noexcept;
    static void SetTextureParameter(GLenum target, GLenum name, GLint param);

    template <typename T>
    static void SetTextureParameter2dNE(GLenum pname, T value) noexcept
    {
        SetTextureParameterNE(GL_TEXTURE_2D, pname, value);
    }

    template <typename T>
    static void SetTextureParameter2d(GLenum pname, T value) noexcept
    {
        SetTextureParameter(GL_TEXTURE_2D, pname, value);
    }

    static void SetTexture2dBorderColorNE(const Vec4f& v) noexcept;
    static void SetTexture2dBorderColor(const Vec4f& v);

    static void SetTexture2dWrapNE(GlTextureWrapAxis wrap, GlTextureWrapMode mode) noexcept;
    static void SetTexture2dWrap(GlTextureWrapAxis wrap, GlTextureWrapMode mode);

    static void SetTexture2dMinFilterNE(GlTextureFilter filter) noexcept;
    static void SetTexture2dMinFilter(GlTextureFilter filter);

    static void SetTexture2dMagFilterNE(GlTextureFilter filter) noexcept;
    static void SetTexture2dMagFilter(GlTextureFilter filter);

    static void BindTextureNE(GLenum target, GLuint texture) noexcept;
    static void BindTexture(GLenum target, GLuint texture);

    static void BindTexture2dNE(GLuint texture) noexcept;
    static void BindTexture2d(GLuint texture);

    static void TexImage2dNE(
        GLenum target,
        size_t level_of_detail,
        GLint internal_format,
        size_t width,
        size_t height,
        GLint data_format,
        GLenum pixel_data_type,
        const void* pixels) noexcept;
    static void TexImage2d(
        GLenum target,
        size_t level_of_detail,
        GLint internal_format,
        size_t width,
        size_t height,
        GLint data_format,
        GLenum pixel_data_type,
        const void* pixels);

    static void GenerateMipmapNE(GLenum target) noexcept;
    static void GenerateMipmap(GLenum target);

    static void GenerateMipmap2dNE() noexcept;
    static void GenerateMipmap2d();

    [[nodiscard]] static std::optional<uint32_t> FindUniformLocationNE(
        GLuint shader_program,
        const char* name) noexcept;
    [[nodiscard]] static std::optional<uint32_t> FindUniformLocation(GLuint shader_program, const char* name);

    [[nodiscard]] static uint32_t GetUniformLocation(GLuint shader_program, const char* name);

    static void PolygonModeNE(GlPolygonMode mode) noexcept;
    static void PolygonMode(GlPolygonMode mode);

    static void PointSizeNE(float size) noexcept;
    static void PointSize(float size);

    static void LineWidthNE(float width) noexcept;
    static void LineWidth(float width);
};

}  // namespace klgl
