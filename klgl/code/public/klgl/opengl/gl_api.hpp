#pragma once

#include <glad/glad.h>

#include <expected>
#include <optional>
#include <span>

#include "EverydayTools/Math/Matrix.hpp"
#include "EverydayTools/Template/TaggedIdentifier.hpp"
#include "enums.hpp"
#include "klgl/opengl/detail/settings.hpp"
#include "open_gl_error.hpp"

namespace klgl::tags
{
struct GlShaderIdTag;
struct GlProgramIdTag;
}  // namespace klgl::tags

namespace klgl
{

using namespace edt::lazy_matrix_aliases;  // NOLINT

using GlShaderId = edt::TaggedIdentifier<tags::GlShaderIdTag, GLuint, 0>;
using GlProgramId = edt::TaggedIdentifier<tags::GlProgramIdTag, GLuint, 0>;

// This class wraps OpenGL calls
// Most methods have a vew version with different suffixes that tell about how error are going to be handled:
// - NE - no error checking. These methods are also marked as noexcept as they promise to not throw exceptions
//   you can still check for errors yourself using OpenGl::GetError function
// - CE - stands for "Consume Errors" checks for errors but does not throw them as exception but returns them as
// OpenGlError (or std::expected with OpenGlError if function has return value)
// - no suffix. Checks for errors and throws an exception with error code, stack trace and raw opengl call description

class OpenGl
{
public:
    struct Internal;

    [[nodiscard]] KLGL_OGL_INLINE static GlError GetError() noexcept;

    [[nodiscard]] KLGL_OGL_INLINE static GLuint GenBuffer() noexcept;
    KLGL_OGL_INLINE static void GenBuffers(const std::span<GLuint>& buffers) noexcept;

    [[nodiscard]] KLGL_OGL_INLINE static GLuint GenVertexArray() noexcept;
    KLGL_OGL_INLINE static void GenVertexArrays(const std::span<GLuint>& arrays) noexcept;

    [[nodiscard]] KLGL_OGL_INLINE static GLuint GenTexture() noexcept;
    KLGL_OGL_INLINE static void GenTextures(const std::span<GLuint>& textures) noexcept;

    KLGL_OGL_INLINE static void BindBufferNE(GlBufferType target, GLuint buffer) noexcept;
    KLGL_OGL_INLINE static void BindBuffer(GlBufferType target, GLuint buffer);

    KLGL_OGL_INLINE static void BindVertexArrayNE(GLuint array) noexcept;
    KLGL_OGL_INLINE static void BindVertexArray(GLuint array);

    KLGL_OGL_INLINE static void
    BufferDataNE(GlBufferType target, GLsizeiptr size, const void* data, GlUsage usage) noexcept;
    KLGL_OGL_INLINE static void BufferData(GlBufferType target, GLsizeiptr size, const void* data, GlUsage usage);

    template <typename T, size_t Extent>
    KLGL_OGL_INLINE static void
    BufferDataNE(GlBufferType target, const std::span<T, Extent>& data, GlUsage usage) noexcept
    {
        BufferDataNE(target, static_cast<GLsizeiptr>(sizeof(T) * data.size()), data.data(), usage);
    }

    template <typename T, size_t Extent>
    KLGL_OGL_INLINE static void BufferData(GlBufferType target, const std::span<T, Extent>& data, GlUsage usage)
    {
        BufferData(target, static_cast<GLsizeiptr>(sizeof(T) * data.size()), data.data(), usage);
    }

    template <typename T, size_t Extent>
    KLGL_OGL_INLINE static void
    BufferDataNE(GlBufferType target, const std::span<const T, Extent>& data, GlUsage usage) noexcept
    {
        BufferDataNE(target, static_cast<GLsizeiptr>(sizeof(T) * data.size()), data.data(), usage);
    }

    template <typename T, size_t Extent>
    KLGL_OGL_INLINE static void
    BufferData(GlBufferType target, const std::span<const T, Extent>& data, GlUsage usage) noexcept
    {
        BufferData(target, static_cast<GLsizeiptr>(sizeof(T) * data.size()), data.data(), usage);
    }

    [[nodiscard]] KLGL_OGL_INLINE static constexpr GLboolean CastBool(bool value) noexcept;

    [[nodiscard]] KLGL_OGL_INLINE static GlShaderId CreateShaderNE(GlShaderType type) noexcept;
    [[nodiscard]] KLGL_OGL_INLINE static std::expected<GlShaderId, OpenGlError> CreateShaderCE(
        GlShaderType type) noexcept;
    [[nodiscard]] KLGL_OGL_INLINE static GlShaderId CreateShader(GlShaderType type);

    KLGL_OGL_INLINE static void DeleteShaderNE(GlShaderId shader) noexcept;
    [[nodiscard]] KLGL_OGL_INLINE static std::optional<OpenGlError> DeleteShaderCE(GlShaderId shader) noexcept;
    KLGL_OGL_INLINE static void DeleteShader(GlShaderId shader);

    KLGL_OGL_INLINE static void ShaderSourceNE(GlShaderId shader, std::span<const std::string_view> sources) noexcept;
    [[nodiscard]] KLGL_OGL_INLINE static std::optional<OpenGlError> ShaderSourceCE(
        GlShaderId shader,
        std::span<const std::string_view> sources) noexcept;
    KLGL_OGL_INLINE static void ShaderSource(GlShaderId shader, std::span<const std::string_view> sources);

    // Note that you still have to check shader status after compilation.
    // These functions return an error if you pass non-existing or deleted shader, for example.
    // But they finish without error if there is some syntax error in your shader.
    KLGL_OGL_INLINE static void CompileShaderNE(GlShaderId shader) noexcept;
    [[nodiscard]] KLGL_OGL_INLINE static std::optional<OpenGlError> CompileShaderCE(GlShaderId shader) noexcept;
    KLGL_OGL_INLINE static void CompileShader(GlShaderId shader);

    [[nodiscard]] KLGL_OGL_INLINE static bool GetShaderCompileStatusNE(GlShaderId shader) noexcept;
    [[nodiscard]] KLGL_OGL_INLINE static std::expected<bool, OpenGlError> GetShaderCompileStatusCE(
        GlShaderId shader) noexcept;
    [[nodiscard]] KLGL_OGL_INLINE static bool GetShaderCompileStatus(GlShaderId shader);

    [[nodiscard]] KLGL_OGL_INLINE static size_t GetShaderLogLengthNE(GlShaderId shader) noexcept;
    [[nodiscard]] KLGL_OGL_INLINE static std::expected<size_t, OpenGlError> GetShaderLogLengthCE(
        GlShaderId shader) noexcept;
    [[nodiscard]] KLGL_OGL_INLINE static size_t GetShaderLogLength(GlShaderId shader);

    [[nodiscard]] KLGL_OGL_INLINE static std::string GetShaderLogNE(GlShaderId shader) noexcept;
    [[nodiscard]] KLGL_OGL_INLINE static std::expected<std::string, OpenGlError> GetShaderLogCE(
        GlShaderId shader) noexcept;
    [[nodiscard]] KLGL_OGL_INLINE static std::string GetShaderLog(GlShaderId shader);

    [[nodiscard]] KLGL_OGL_INLINE static GlProgramId CreateProgramNE() noexcept;
    [[nodiscard]] KLGL_OGL_INLINE static std::expected<GlProgramId, OpenGlError> CreateProgramCE() noexcept;
    [[nodiscard]] KLGL_OGL_INLINE static GlProgramId CreateProgram();

    KLGL_OGL_INLINE static void AttachShaderNE(GlProgramId program, GlShaderId shader) noexcept;
    [[nodiscard]] KLGL_OGL_INLINE static std::optional<OpenGlError> AttachShaderCE(
        GlProgramId program,
        GlShaderId shader) noexcept;
    KLGL_OGL_INLINE static void AttachShader(GlProgramId program, GlShaderId shader);

    // Same as for shader compilation: check link status afterwards!
    KLGL_OGL_INLINE static void LinkProgramNE(GlProgramId program) noexcept;
    [[nodiscard]] KLGL_OGL_INLINE static std::optional<OpenGlError> LinkProgramCE(GlProgramId program) noexcept;
    KLGL_OGL_INLINE static void LinkProgram(GlProgramId program);

    [[nodiscard]] KLGL_OGL_INLINE static bool GetProgramLinkStatusNE(GlProgramId program) noexcept;
    [[nodiscard]] KLGL_OGL_INLINE static std::expected<bool, OpenGlError> GetProgramLinkStatusCE(
        GlProgramId program) noexcept;
    [[nodiscard]] KLGL_OGL_INLINE static bool GetProgramLinkStatus(GlProgramId program);

    [[nodiscard]] KLGL_OGL_INLINE static size_t GetProgramLogLengthNE(GlProgramId program) noexcept;
    [[nodiscard]] KLGL_OGL_INLINE static std::expected<size_t, OpenGlError> GetProgramLogLengthCE(
        GlProgramId program) noexcept;
    [[nodiscard]] KLGL_OGL_INLINE static size_t GetProgramLogLength(GlProgramId program);

    [[nodiscard]] KLGL_OGL_INLINE static std::string GetProgramLogNE(GlProgramId program) noexcept;
    [[nodiscard]] KLGL_OGL_INLINE static std::expected<std::string, OpenGlError> GetProgramLogCE(
        GlProgramId program) noexcept;
    [[nodiscard]] KLGL_OGL_INLINE static std::string GetProgramLog(GlProgramId program);

    KLGL_OGL_INLINE static void UseProgramNE(GlProgramId program) noexcept;
    [[nodiscard]] KLGL_OGL_INLINE static std::optional<OpenGlError> UseProgramCE(GlProgramId program) noexcept;
    KLGL_OGL_INLINE static void UseProgram(GlProgramId program);

    KLGL_OGL_INLINE static void DeleteProgramNE(GlProgramId program) noexcept;
    [[nodiscard]] KLGL_OGL_INLINE static std::optional<OpenGlError> DeleteProgramCE(GlProgramId program) noexcept;
    KLGL_OGL_INLINE static void DeleteProgram(GlProgramId program);

    KLGL_OGL_INLINE static void VertexAttribPointerNE(
        GLuint index,
        size_t size,
        GlVertexAttribComponentType type,
        bool normalized,
        size_t stride,
        const void* pointer) noexcept;
    KLGL_OGL_INLINE static std::optional<OpenGlError> VertexAttribPointerCE(
        GLuint index,
        size_t size,
        GlVertexAttribComponentType type,
        bool normalized,
        size_t stride,
        const void* pointer) noexcept;
    KLGL_OGL_INLINE static void VertexAttribPointer(
        GLuint index,
        size_t size,
        GlVertexAttribComponentType type,
        bool normalized,
        size_t stride,
        const void* pointer);

    KLGL_OGL_INLINE static void EnableVertexAttribArrayNE(GLuint index) noexcept;
    [[nodiscard]] KLGL_OGL_INLINE static std::optional<OpenGlError> EnableVertexAttribArrayCE(GLuint index) noexcept;
    KLGL_OGL_INLINE static void EnableVertexAttribArray(GLuint index);

    KLGL_OGL_INLINE static void EnableDepthTestNE() noexcept;
    [[nodiscard]] KLGL_OGL_INLINE static std::optional<OpenGlError> EnableDepthTestCE() noexcept;
    KLGL_OGL_INLINE static void EnableDepthTest();

    KLGL_OGL_INLINE static void EnableBlendingNE() noexcept;
    [[nodiscard]] KLGL_OGL_INLINE static std::optional<OpenGlError> EnableBlendingCE() noexcept;
    KLGL_OGL_INLINE static void EnableBlending();

    KLGL_OGL_INLINE static void ViewportNE(GLint x, GLint y, GLsizei width, GLsizei height) noexcept;
    [[nodiscard]] KLGL_OGL_INLINE static std::optional<OpenGlError>
    ViewportCE(GLint x, GLint y, GLsizei width, GLsizei height) noexcept;
    KLGL_OGL_INLINE static void Viewport(GLint x, GLint y, GLsizei width, GLsizei height);

    KLGL_OGL_INLINE static void SetClearColorNE(GLfloat r, GLfloat g, GLfloat b, GLfloat a) noexcept;
    [[nodiscard]] KLGL_OGL_INLINE static std::optional<OpenGlError>
    SetClearColorCE(GLfloat r, GLfloat g, GLfloat b, GLfloat a) noexcept;
    KLGL_OGL_INLINE static void SetClearColor(GLfloat r, GLfloat g, GLfloat b, GLfloat a);

    KLGL_OGL_INLINE static void SetClearColorNE(const Vec4f& color) noexcept;
    [[nodiscard]] KLGL_OGL_INLINE static std::optional<OpenGlError> SetClearColorCE(const Vec4f& color) noexcept;
    KLGL_OGL_INLINE static void SetClearColor(const Vec4f& color);

    KLGL_OGL_INLINE static void ClearNE(GLbitfield mask) noexcept;
    [[nodiscard]] KLGL_OGL_INLINE static std::optional<OpenGlError> ClearCE(GLbitfield mask) noexcept;
    KLGL_OGL_INLINE static void Clear(GLbitfield mask) noexcept;

    KLGL_OGL_INLINE static void DrawElementsNE(
        GlPrimitiveType mode,
        size_t num,
        GlIndexBufferElementType indices_type,
        const void* indices) noexcept;
    [[nodiscard]] KLGL_OGL_INLINE static std::optional<OpenGlError> DrawElementsCE(
        GlPrimitiveType mode,
        size_t num,
        GlIndexBufferElementType indices_type,
        const void* indices) noexcept;
    KLGL_OGL_INLINE static void
    DrawElements(GlPrimitiveType mode, size_t num, GlIndexBufferElementType indices_type, const void* indices);

    KLGL_OGL_INLINE static void DrawElementsInstancedNE(
        GlPrimitiveType mode,
        size_t num,
        GlIndexBufferElementType indices_type,
        const void* indices,
        size_t num_instances) noexcept;
    [[nodiscard]] KLGL_OGL_INLINE static std::optional<OpenGlError> DrawElementsInstancedCE(
        GlPrimitiveType mode,
        size_t num,
        GlIndexBufferElementType indices_type,
        const void* indices,
        size_t num_instances) noexcept;
    KLGL_OGL_INLINE static void DrawElementsInstanced(
        GlPrimitiveType mode,
        size_t num,
        GlIndexBufferElementType indices_type,
        const void* indices,
        size_t num_instances);

    KLGL_OGL_INLINE static void SetUniformNE(uint32_t location, const float& f) noexcept;
    [[nodiscard]] KLGL_OGL_INLINE static std::optional<OpenGlError> SetUniformCE(
        uint32_t location,
        const float& f) noexcept;
    KLGL_OGL_INLINE static void SetUniform(uint32_t location, const float& f);

    KLGL_OGL_INLINE static void SetUniformNE(uint32_t location, const Vec2f& v) noexcept;
    [[nodiscard]] KLGL_OGL_INLINE static std::optional<OpenGlError> SetUniformCE(
        uint32_t location,
        const Vec2f& v) noexcept;
    KLGL_OGL_INLINE static void SetUniform(uint32_t location, const Vec2f& v);

    KLGL_OGL_INLINE static void SetUniformNE(uint32_t location, const Vec3f& v) noexcept;
    [[nodiscard]] KLGL_OGL_INLINE static std::optional<OpenGlError> SetUniformCE(
        uint32_t location,
        const Vec3f& v) noexcept;
    KLGL_OGL_INLINE static void SetUniform(uint32_t location, const Vec3f& v);

    KLGL_OGL_INLINE static void SetUniformNE(uint32_t location, const Vec4f& v) noexcept;
    [[nodiscard]] KLGL_OGL_INLINE static std::optional<OpenGlError> SetUniformCE(
        uint32_t location,
        const Vec4f& v) noexcept;
    KLGL_OGL_INLINE static void SetUniform(uint32_t location, const Vec4f& v);

    KLGL_OGL_INLINE static void SetUniformNE(uint32_t location, const Mat3f& m, bool transpose = false) noexcept;
    [[nodiscard]] KLGL_OGL_INLINE static std::optional<OpenGlError>
    SetUniformCE(uint32_t location, const Mat3f& m, bool transpose = false) noexcept;
    KLGL_OGL_INLINE static void SetUniform(uint32_t location, const Mat3f& m, bool transpose = false);

    KLGL_OGL_INLINE static void SetUniformNE(uint32_t location, const Mat4f& m, bool transpose = false) noexcept;
    [[nodiscard]] KLGL_OGL_INLINE static std::optional<OpenGlError>
    SetUniformCE(uint32_t location, const Mat4f& m, bool transpose = false) noexcept;
    KLGL_OGL_INLINE static void SetUniform(uint32_t location, const Mat4f& m, bool transpose = false);

    // Specifies the texture comparison mode for currently bound depth textures.
    // (a texture whose internal format is GL_DEPTH_COMPONENT_*)
    KLGL_OGL_INLINE static void SetDepthTextureCompareModeNE(
        GlTextureParameterTarget target,
        GlDepthTextureCompareMode mode) noexcept;
    KLGL_OGL_INLINE static void SetDepthTextureCompareMode(
        GlTextureParameterTarget target,
        GlDepthTextureCompareMode mode);

    KLGL_OGL_INLINE static void SetDepthTextureCompareFunctionNE(
        GlTextureParameterTarget target,
        GlDepthTextureCompareFunction function) noexcept;
    KLGL_OGL_INLINE static void SetDepthTextureCompareFunction(
        GlTextureParameterTarget target,
        GlDepthTextureCompareFunction function);

    KLGL_OGL_INLINE static void
    SetTextureParameterNE(GlTextureParameterTarget target, GLenum pname, const GLfloat* value) noexcept;
    KLGL_OGL_INLINE static void
    SetTextureParameter(GlTextureParameterTarget target, GLenum pname, const GLfloat* value);

    KLGL_OGL_INLINE static void
    SetTextureParameterNE(GlTextureParameterTarget target, GLenum name, GLint param) noexcept;
    KLGL_OGL_INLINE static void SetTextureParameter(GlTextureParameterTarget target, GLenum name, GLint param);

    // Specifies the index of the lowest defined mipmap level. The initial value is 0.
    KLGL_OGL_INLINE static void SetTextureBaseLevelNE(GlTextureParameterTarget target, size_t level) noexcept;
    KLGL_OGL_INLINE static void SetTextureBaseLevel(GlTextureParameterTarget target, size_t level);

    // The data in params specifies four values that define the border values that should be used for border texels. If
    // a texel is sampled from the border of the texture, the values of GL_TEXTURE_BORDER_COLOR are interpreted as an
    // RGBA color to match the texture's internal format and substituted for the non-existent texel data. If the texture
    // contains depth components, the first component of GL_TEXTURE_BORDER_COLOR is interpreted as a depth value. The
    // initial value is (0.0,0.0,0.0,0.0).
    // If the values are specified with store_as_integer = true the values are stored unmodified with an internal data
    // type of integer. Otherwise, they are converted to floating point with the following equation: f=2c+1/2^b−1.
    KLGL_OGL_INLINE static void SetTextureBorderColorNE(
        GlTextureParameterTarget target,
        std::span<const GLint, 4> color,
        bool store_as_integer) noexcept;
    KLGL_OGL_INLINE static void
    SetTextureBorderColor(GlTextureParameterTarget target, std::span<const GLint, 4> color, bool store_as_integer);
    KLGL_OGL_INLINE static void SetTextureBorderColorNE(
        GlTextureParameterTarget target,
        std::span<const GLuint, 4> color) noexcept;
    KLGL_OGL_INLINE static void SetTextureBorderColor(
        GlTextureParameterTarget target,
        std::span<const GLuint, 4> color);
    KLGL_OGL_INLINE static void SetTextureBorderColorNE(
        GlTextureParameterTarget target,
        std::span<const GLfloat, 4> color) noexcept;
    KLGL_OGL_INLINE static void SetTextureBorderColor(
        GlTextureParameterTarget target,
        std::span<const GLfloat, 4> color);

    // Specifies a fixed bias value that is to be added to the level-of-detail parameter for the texture before texture
    // sampling. The specified value is added to the shader-supplied bias value (if any) and subsequently clamped into
    // the implementation-defined range [−biasmax,biasmax], where biasmax is the value of the implementation defined
    // constant GL_MAX_TEXTURE_LOD_BIAS. The initial value is 0.0.
    KLGL_OGL_INLINE static void SetTextureLODBiasNE(GlTextureParameterTarget target, float bias) noexcept;
    KLGL_OGL_INLINE static void SetTextureLODBias(GlTextureParameterTarget target, float bias);

    KLGL_OGL_INLINE static void
    SetTextureWrapNE(GlTextureParameterTarget target, GlTextureWrapAxis wrap, GlTextureWrapMode mode) noexcept;
    KLGL_OGL_INLINE static void
    SetTextureWrap(GlTextureParameterTarget target, GlTextureWrapAxis wrap, GlTextureWrapMode mode);

    KLGL_OGL_INLINE static void SetTextureMinFilterNE(GlTextureParameterTarget target, GlTextureFilter filter) noexcept;
    KLGL_OGL_INLINE static void SetTextureMinFilter(GlTextureParameterTarget target, GlTextureFilter filter);

    KLGL_OGL_INLINE static void SetTextureMagFilterNE(GlTextureParameterTarget target, GlTextureFilter filter) noexcept;
    KLGL_OGL_INLINE static void SetTextureMagFilter(GlTextureParameterTarget target, GlTextureFilter filter);

    KLGL_OGL_INLINE static void BindTextureNE(GLenum target, GLuint texture) noexcept;
    KLGL_OGL_INLINE static void BindTexture(GLenum target, GLuint texture);

    KLGL_OGL_INLINE static void BindTexture2dNE(GLuint texture) noexcept;
    KLGL_OGL_INLINE static void BindTexture2d(GLuint texture);

    KLGL_OGL_INLINE static void TexImage2dNE(
        GLenum target,
        size_t level_of_detail,
        GLint internal_format,
        size_t width,
        size_t height,
        GLint data_format,
        GLenum pixel_data_type,
        const void* pixels) noexcept;
    KLGL_OGL_INLINE static void TexImage2d(
        GLenum target,
        size_t level_of_detail,
        GLint internal_format,
        size_t width,
        size_t height,
        GLint data_format,
        GLenum pixel_data_type,
        const void* pixels);

    KLGL_OGL_INLINE static void GenerateMipmapNE(GLenum target) noexcept;
    KLGL_OGL_INLINE static void GenerateMipmap(GLenum target);

    KLGL_OGL_INLINE static void GenerateMipmap2dNE() noexcept;
    KLGL_OGL_INLINE static void GenerateMipmap2d();

    [[nodiscard]] KLGL_OGL_INLINE static std::optional<uint32_t> FindUniformLocationNE(
        GLuint shader_program,
        const char* name) noexcept;
    [[nodiscard]] KLGL_OGL_INLINE static std::optional<uint32_t> FindUniformLocation(
        GLuint shader_program,
        const char* name);

    [[nodiscard]] KLGL_OGL_INLINE static uint32_t GetUniformLocation(GLuint shader_program, const char* name);

    KLGL_OGL_INLINE static void PolygonModeNE(GlPolygonMode mode) noexcept;
    KLGL_OGL_INLINE static void PolygonMode(GlPolygonMode mode);

    KLGL_OGL_INLINE static void PointSizeNE(float size) noexcept;
    KLGL_OGL_INLINE static void PointSize(float size);

    KLGL_OGL_INLINE static void LineWidthNE(float width) noexcept;
    KLGL_OGL_INLINE static void LineWidth(float width);
};

}  // namespace klgl

#if KLGL_INLINE_OPENGL_WRAPPERS
#include "detail/gl_api_impl.hpp"
#endif
