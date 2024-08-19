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
        GlVertexAttribComponentType type,
        bool normalized,
        size_t stride,
        const void* pointer) noexcept;
    static void VertexAttribPointer(
        GLuint index,
        size_t size,
        GlVertexAttribComponentType type,
        bool normalized,
        size_t stride,
        const void* pointer);

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

    static void DrawElementsNE(
        GlPrimitiveType mode,
        size_t num,
        GlIndexBufferElementType indices_type,
        const void* indices) noexcept;
    static void
    DrawElements(GlPrimitiveType mode, size_t num, GlIndexBufferElementType indices_type, const void* indices);

    static void DrawElementsInstancedNE(
        GlPrimitiveType mode,
        size_t num,
        GlIndexBufferElementType indices_type,
        const void* indices,
        size_t num_instances) noexcept;
    static void DrawElementsInstanced(
        GlPrimitiveType mode,
        size_t num,
        GlIndexBufferElementType indices_type,
        const void* indices,
        size_t num_instances);

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

    // Specifies the texture comparison mode for currently bound depth textures.
    // (a texture whose internal format is GL_DEPTH_COMPONENT_*)
    static void SetDepthTextureCompareModeNE(GlTextureParameterTarget target, GlDepthTextureCompareMode mode) noexcept;
    static void SetDepthTextureCompareMode(GlTextureParameterTarget target, GlDepthTextureCompareMode mode);

    static void SetDepthTextureCompareFunctionNE(
        GlTextureParameterTarget target,
        GlDepthTextureCompareFunction function) noexcept;
    static void SetDepthTextureCompareFunction(GlTextureParameterTarget target, GlDepthTextureCompareFunction function);

    static void SetTextureParameterNE(GlTextureParameterTarget target, GLenum pname, const GLfloat* value) noexcept;
    static void SetTextureParameter(GlTextureParameterTarget target, GLenum pname, const GLfloat* value);

    static void SetTextureParameterNE(GlTextureParameterTarget target, GLenum name, GLint param) noexcept;
    static void SetTextureParameter(GlTextureParameterTarget target, GLenum name, GLint param);

    // Specifies the index of the lowest defined mipmap level. The initial value is 0.
    static void SetTextureBaseLevelNE(GlTextureParameterTarget target, size_t level) noexcept;
    static void SetTextureBaseLevel(GlTextureParameterTarget target, size_t level);

    // The data in params specifies four values that define the border values that should be used for border texels. If
    // a texel is sampled from the border of the texture, the values of GL_TEXTURE_BORDER_COLOR are interpreted as an
    // RGBA color to match the texture's internal format and substituted for the non-existent texel data. If the texture
    // contains depth components, the first component of GL_TEXTURE_BORDER_COLOR is interpreted as a depth value. The
    // initial value is (0.0,0.0,0.0,0.0).
    // If the values are specified with store_as_integer = true the values are stored unmodified with an internal data
    // type of integer. Otherwise, they are converted to floating point with the following equation: f=2c+1/2^b−1.
    static void SetTextureBorderColorNE(
        GlTextureParameterTarget target,
        std::span<const GLint, 4> color,
        bool store_as_integer) noexcept;
    static void
    SetTextureBorderColor(GlTextureParameterTarget target, std::span<const GLint, 4> color, bool store_as_integer);
    static void SetTextureBorderColorNE(GlTextureParameterTarget target, std::span<const GLuint, 4> color) noexcept;
    static void SetTextureBorderColor(GlTextureParameterTarget target, std::span<const GLuint, 4> color);
    static void SetTextureBorderColorNE(GlTextureParameterTarget target, std::span<const GLfloat, 4> color) noexcept;
    static void SetTextureBorderColor(GlTextureParameterTarget target, std::span<const GLfloat, 4> color);

    // Specifies a fixed bias value that is to be added to the level-of-detail parameter for the texture before texture
    // sampling. The specified value is added to the shader-supplied bias value (if any) and subsequently clamped into
    // the implementation-defined range [−biasmax,biasmax], where biasmax is the value of the implementation defined
    // constant GL_MAX_TEXTURE_LOD_BIAS. The initial value is 0.0.
    static void SetTextureLODBiasNE(GlTextureParameterTarget target, float bias) noexcept;
    static void SetTextureLODBias(GlTextureParameterTarget target, float bias);

    template <typename T>
    static void SetTextureParameter2dNE(GLenum pname, T value) noexcept
    {
        SetTextureParameterNE(GlTextureParameterTarget::Texture2d, pname, value);
    }

    template <typename T>
    static void SetTextureParameter2d(GLenum pname, T value) noexcept
    {
        SetTextureParameter(GlTextureParameterTarget::Texture2d, pname, value);
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
