#pragma once

#include <glad/glad.h>

#include <expected>
#include <optional>
#include <span>

#include "EverydayTools/Math/Matrix.hpp"
#include "enums.hpp"
#include "identifiers.hpp"
#include "klgl/opengl/detail/settings.hpp"
#include "open_gl_error.hpp"

namespace klgl
{

using namespace edt::lazy_matrix_aliases;  // NOLINT

// This class wraps OpenGL calls
// Most methods have a vew version with different suffixes that tell about how error are going to be handled:
// - NE - no error checking. These methods are also marked as noexcept as they promise to not throw exceptions
//   you can still check for errors yourself using OpenGl::GetError function
// - CE - stands for "Consume Errors" checks for errors but does not throw them as exception but returns them as
// OpenGlError (or std::expected with OpenGlError if function has return value)
// - no suffix. Checks for errors and throws an exception with error code, stack trace and raw opengl call description

class OpenGl
{
    struct Internal;

public:
    [[nodiscard]] KLGL_OGL_INLINE static GlError GetError() noexcept;

    /************************************************** Buffers *******************************************************/

    KLGL_OGL_INLINE static void GenBuffersNE(const std::span<GlBufferId>& buffers) noexcept;
    [[nodiscard]] KLGL_OGL_INLINE static std::optional<OpenGlError> GenBuffersCE(
        const std::span<GlBufferId>& buffers) noexcept;
    KLGL_OGL_INLINE static void GenBuffers(const std::span<GlBufferId>& buffers);

    [[nodiscard]] KLGL_OGL_INLINE static GlBufferId GenBufferNE() noexcept;
    [[nodiscard]] KLGL_OGL_INLINE static std::expected<GlBufferId, OpenGlError> GenBufferCE() noexcept;
    [[nodiscard]] KLGL_OGL_INLINE static GlBufferId GenBuffer();

    KLGL_OGL_INLINE static void BindBufferNE(GlBufferType target, GlBufferId buffer) noexcept;
    [[nodiscard]] KLGL_OGL_INLINE static std::optional<OpenGlError> BindBufferCE(
        GlBufferType target,
        GlBufferId buffer) noexcept;
    KLGL_OGL_INLINE static void BindBuffer(GlBufferType target, GlBufferId buffer);

    // This overload allows you to initalize buffer without data
    KLGL_OGL_INLINE static void BufferDataNE(GlBufferType target, size_t buffer_size, GlUsage usage) noexcept;
    [[nodiscard]] KLGL_OGL_INLINE static std::optional<OpenGlError>
    BufferDataCE(GlBufferType target, size_t buffer_size, GlUsage usage) noexcept;
    KLGL_OGL_INLINE static void BufferData(GlBufferType target, size_t buffer_size, GlUsage usage);

    KLGL_OGL_INLINE static void
    BufferDataNE(GlBufferType target, std::span<const uint8_t> data, GlUsage usage) noexcept;
    [[nodiscard]] KLGL_OGL_INLINE static std::optional<OpenGlError>
    BufferDataCE(GlBufferType target, std::span<const uint8_t> data, GlUsage usage) noexcept;
    KLGL_OGL_INLINE static void BufferData(GlBufferType target, std::span<const uint8_t> data, GlUsage usage);

    template <typename T, size_t Extent>
        requires(!std::same_as<std::remove_const_t<T>, uint8_t>)
    KLGL_OGL_INLINE
        static void BufferDataNE(GlBufferType target, const std::span<T, Extent>& data, GlUsage usage) noexcept
    {
        std::span bytes{reinterpret_cast<const uint8_t*>(data.data()), data.size_bytes()};  // NOLINT
        BufferDataNE(target, bytes, usage);
    }

    template <typename T, size_t Extent>
        requires(!std::same_as<std::remove_const_t<T>, uint8_t>)
    [[nodiscard]] KLGL_OGL_INLINE static std::optional<OpenGlError> BufferDataCE(
        GlBufferType target,
        const std::span<T, Extent>& data,
        GlUsage usage) noexcept
    {
        std::span bytes{reinterpret_cast<const uint8_t*>(data.data()), data.size_bytes()};  // NOLINT
        return BufferDataCE(target, bytes, usage);
    }

    template <typename T, size_t Extent>
        requires(!std::same_as<std::remove_const_t<T>, uint8_t>)
    KLGL_OGL_INLINE
        static void BufferData(GlBufferType target, const std::span<T, Extent>& data, GlUsage usage) noexcept
    {
        std::span bytes{reinterpret_cast<const uint8_t*>(data.data()), data.size_bytes()};  // NOLINT
        return BufferData(target, bytes, usage);
    }

    // Redefine some or all of the data store for the specified buffer object. Data starting at offset offset and
    // extending for size bytes is copied to the data store from the memory pointed to by data. offset and size must
    // define a range lying entirely within the buffer object's data store.
    // When replacing the entire data store, consider using glBufferSubData rather than completely recreating the data
    // store with glBufferData. This avoids the cost of reallocating the data store.
    KLGL_OGL_INLINE static void
    BufferSubDataNE(GlBufferType target, size_t offset_elements, std::span<const uint8_t> data) noexcept;
    [[nodiscard]] KLGL_OGL_INLINE static std::optional<OpenGlError>
    BufferSubDataCE(GlBufferType target, size_t offset_elements, std::span<const uint8_t> data) noexcept;
    KLGL_OGL_INLINE static void
    BufferSubData(GlBufferType target, size_t offset_elements, std::span<const uint8_t> data);

    KLGL_OGL_INLINE static void DeleteBufferNE(GlBufferId buffer) noexcept;
    [[nodiscard]] KLGL_OGL_INLINE static std::optional<OpenGlError> DeleteBufferCE(GlBufferId buffer) noexcept;
    KLGL_OGL_INLINE static void DeleteBuffer(GlBufferId buffer);

    template <typename T, size_t Extent>
        requires(!std::same_as<std::remove_const_t<T>, uint8_t>)
    KLGL_OGL_INLINE static void BufferSubDataNE(
        GlBufferType target,
        size_t offset_elements,
        const std::span<T, Extent>& data) noexcept
    {
        std::span bytes{reinterpret_cast<const uint8_t*>(data.data()), data.size_bytes()};  // NOLINT
        BufferSubDataNE(target, offset_elements * sizeof(T), bytes);
    }

    template <typename T, size_t Extent>
        requires(!std::same_as<std::remove_const_t<T>, uint8_t>)
    KLGL_OGL_INLINE static std::optional<OpenGlError> BufferSubDataCE(
        GlBufferType target,
        size_t offset_elements,
        const std::span<T, Extent>& data) noexcept
    {
        std::span bytes{reinterpret_cast<const uint8_t*>(data.data()), data.size_bytes()};  // NOLINT
        return BufferSubDataCE(target, offset_elements * sizeof(T), bytes);
    }

    template <typename T, size_t Extent>
        requires(!std::same_as<std::remove_const_t<T>, uint8_t>)
    KLGL_OGL_INLINE static void BufferSubData(
        GlBufferType target,
        size_t offset_elements,
        const std::span<T, Extent>& data) noexcept
    {
        std::span bytes{reinterpret_cast<const uint8_t*>(data.data()), data.size_bytes()};  // NOLINT
        BufferSubData(target, offset_elements * sizeof(T), bytes);
    }

    /*********************************************** Vertex Arrays ****************************************************/

    KLGL_OGL_INLINE static void GenVertexArraysNE(const std::span<GlVertexArrayId>& arrays) noexcept;
    [[nodiscard]] KLGL_OGL_INLINE static std::optional<OpenGlError> GenVertexArraysCE(
        const std::span<GlVertexArrayId>& arrays) noexcept;
    KLGL_OGL_INLINE static void GenVertexArrays(const std::span<GlVertexArrayId>& arrays);

    [[nodiscard]] KLGL_OGL_INLINE static GlVertexArrayId GenVertexArrayNE() noexcept;
    [[nodiscard]] KLGL_OGL_INLINE static std::expected<GlVertexArrayId, OpenGlError> GenVertexArrayCE() noexcept;
    [[nodiscard]] KLGL_OGL_INLINE static GlVertexArrayId GenVertexArray();

    KLGL_OGL_INLINE static void BindVertexArrayNE(GlVertexArrayId array) noexcept;
    [[nodiscard]] KLGL_OGL_INLINE static std::optional<OpenGlError> BindVertexArrayCE(GlVertexArrayId array) noexcept;
    KLGL_OGL_INLINE static void BindVertexArray(GlVertexArrayId array);

    KLGL_OGL_INLINE static void DeleteVertexArrayNE(GlVertexArrayId array) noexcept;
    [[nodiscard]] KLGL_OGL_INLINE static std::optional<OpenGlError> DeleteVertexArrayCE(GlVertexArrayId array) noexcept;
    KLGL_OGL_INLINE static void DeleteVertexArray(GlVertexArrayId array);

    /************************************************* Textures *******************************************************/

    KLGL_OGL_INLINE static void GenTexturesNE(const std::span<GlTextureId>& textures) noexcept;
    [[nodiscard]] KLGL_OGL_INLINE static std::optional<OpenGlError> GenTexturesCE(
        const std::span<GlTextureId>& textures) noexcept;
    KLGL_OGL_INLINE static void GenTextures(const std::span<GlTextureId>& textures);

    [[nodiscard]] KLGL_OGL_INLINE static GlTextureId GenTextureNE() noexcept;
    [[nodiscard]] KLGL_OGL_INLINE static std::expected<GlTextureId, OpenGlError> GenTextureCE() noexcept;
    [[nodiscard]] KLGL_OGL_INLINE static GlTextureId GenTexture();

    KLGL_OGL_INLINE static void BindTextureNE(GlTargetTextureType target, GlTextureId texture) noexcept;
    [[nodiscard]] KLGL_OGL_INLINE static std::optional<OpenGlError> BindtextureCE(
        GlTargetTextureType target,
        GlTextureId texture) noexcept;
    KLGL_OGL_INLINE static void BindTexture(GlTargetTextureType target, GlTextureId texture);

    // Specifies the index of the lowest defined mipmap level. The initial value is 0.
    KLGL_OGL_INLINE static void SetTextureBaseLevelNE(GlTargetTextureType target, size_t level) noexcept;
    [[nodiscard]] KLGL_OGL_INLINE static std::optional<OpenGlError> SetTextureBaseLevelCE(
        GlTargetTextureType target,
        size_t level) noexcept;
    KLGL_OGL_INLINE static void SetTextureBaseLevel(GlTargetTextureType target, size_t level);

    // Specifies the texture comparison mode for currently bound depth textures.
    // (a texture whose internal format is GL_DEPTH_COMPONENT_*)
    KLGL_OGL_INLINE static void SetDepthTextureCompareModeNE(
        GlTargetTextureType target,
        GlDepthTextureCompareMode mode) noexcept;
    [[nodiscard]] KLGL_OGL_INLINE static std::optional<OpenGlError> SetDepthTextureCompareModeCE(
        GlTargetTextureType target,
        GlDepthTextureCompareMode mode) noexcept;
    KLGL_OGL_INLINE static void SetDepthTextureCompareMode(GlTargetTextureType target, GlDepthTextureCompareMode mode);

    KLGL_OGL_INLINE static void SetDepthTextureCompareFunctionNE(
        GlTargetTextureType target,
        GlDepthTextureCompareFunction function) noexcept;
    [[nodiscard]] KLGL_OGL_INLINE static std::optional<OpenGlError> SetDepthTextureCompareFunctionCE(
        GlTargetTextureType target,
        GlDepthTextureCompareFunction function) noexcept;
    KLGL_OGL_INLINE static void SetDepthTextureCompareFunction(
        GlTargetTextureType target,
        GlDepthTextureCompareFunction function);

    // The data in params specifies four values that define the border values that should be used for border texels. If
    // a texel is sampled from the border of the texture, the values of GL_TEXTURE_BORDER_COLOR are interpreted as an
    // RGBA color to match the texture's internal format and substituted for the non-existent texel data. If the texture
    // contains depth components, the first component of GL_TEXTURE_BORDER_COLOR is interpreted as a depth value. The
    // initial value is (0.0,0.0,0.0,0.0).
    // If the values are specified with store_as_integer = true the values are stored unmodified with an internal data
    // type of integer. Otherwise, they are converted to floating point with the following equation: f=2c+1/2^b−1.
    KLGL_OGL_INLINE static void SetTextureBorderColorNE(
        GlTargetTextureType target,
        std::span<const GLint, 4> color,
        bool store_as_integer) noexcept;
    [[nodiscard]] KLGL_OGL_INLINE static std::optional<OpenGlError> SetTextureBorderColorCE(
        GlTargetTextureType target,
        std::span<const GLint, 4> color,
        bool store_as_integer) noexcept;
    KLGL_OGL_INLINE static void
    SetTextureBorderColor(GlTargetTextureType target, std::span<const GLint, 4> color, bool store_as_integer);

    KLGL_OGL_INLINE static void SetTextureBorderColorNE(
        GlTargetTextureType target,
        std::span<const GLuint, 4> color) noexcept;
    [[nodiscard]] KLGL_OGL_INLINE static std::optional<OpenGlError> SetTextureBorderColorCE(
        GlTargetTextureType target,
        std::span<const GLuint, 4> color) noexcept;
    KLGL_OGL_INLINE static void SetTextureBorderColor(GlTargetTextureType target, std::span<const GLuint, 4> color);

    KLGL_OGL_INLINE static void SetTextureBorderColorNE(
        GlTargetTextureType target,
        std::span<const GLfloat, 4> color) noexcept;
    [[nodiscard]] KLGL_OGL_INLINE static std::optional<OpenGlError> SetTextureBorderColorCE(
        GlTargetTextureType target,
        std::span<const GLfloat, 4> color) noexcept;
    KLGL_OGL_INLINE static void SetTextureBorderColor(GlTargetTextureType target, std::span<const GLfloat, 4> color);

    // Specifies a fixed bias value that is to be added to the level-of-detail parameter for the texture before texture
    // sampling. The specified value is added to the shader-supplied bias value (if any) and subsequently clamped into
    // the implementation-defined range [−biasmax,biasmax], where biasmax is the value of the implementation defined
    // constant GL_MAX_TEXTURE_LOD_BIAS. The initial value is 0.0.
    KLGL_OGL_INLINE static void SetTextureLODBiasNE(GlTargetTextureType target, float bias) noexcept;
    [[nodiscard]] KLGL_OGL_INLINE static std::optional<OpenGlError> SetTextureLODBiasCE(
        GlTargetTextureType target,
        float bias) noexcept;
    KLGL_OGL_INLINE static void SetTextureLODBias(GlTargetTextureType target, float bias);

    KLGL_OGL_INLINE static void
    SetTextureWrapNE(GlTargetTextureType target, GlTextureWrapAxis wrap, GlTextureWrapMode mode) noexcept;
    [[nodiscard]] KLGL_OGL_INLINE static std::optional<OpenGlError>
    SetTextureWrapCE(GlTargetTextureType target, GlTextureWrapAxis wrap, GlTextureWrapMode mode) noexcept;
    KLGL_OGL_INLINE static void
    SetTextureWrap(GlTargetTextureType target, GlTextureWrapAxis wrap, GlTextureWrapMode mode);

    KLGL_OGL_INLINE static void SetTextureMinFilterNE(GlTargetTextureType target, GlTextureFilter filter) noexcept;
    [[nodiscard]] KLGL_OGL_INLINE static std::optional<OpenGlError> SetTextureMinFilterCE(
        GlTargetTextureType target,
        GlTextureFilter filter) noexcept;
    KLGL_OGL_INLINE static void SetTextureMinFilter(GlTargetTextureType target, GlTextureFilter filter);

    KLGL_OGL_INLINE static void SetTextureMagFilterNE(GlTargetTextureType target, GlTextureFilter filter) noexcept;
    [[nodiscard]] KLGL_OGL_INLINE static std::optional<OpenGlError> SetTextureMagFilterCE(
        GlTargetTextureType target,
        GlTextureFilter filter) noexcept;
    KLGL_OGL_INLINE static void SetTextureMagFilter(GlTargetTextureType target, GlTextureFilter filter);

    KLGL_OGL_INLINE static void TexImage2dNE(
        GlTargetTextureType target,
        size_t level_of_detail,
        GLint internal_format,
        size_t width,
        size_t height,
        GLint data_format,
        GLenum pixel_data_type,
        const void* pixels) noexcept;
    [[nodiscard]] KLGL_OGL_INLINE static std::optional<OpenGlError> TexImage2dCE(
        GlTargetTextureType target,
        size_t level_of_detail,
        GLint internal_format,
        size_t width,
        size_t height,
        GLint data_format,
        GLenum pixel_data_type,
        const void* pixels) noexcept;
    KLGL_OGL_INLINE static void TexImage2d(
        GlTargetTextureType target,
        size_t level_of_detail,
        GLint internal_format,
        size_t width,
        size_t height,
        GLint data_format,
        GLenum pixel_data_type,
        const void* pixels);

    KLGL_OGL_INLINE static void DeleteTextureNE(GlTextureId texture) noexcept;
    [[nodiscard]] KLGL_OGL_INLINE static std::optional<OpenGlError> DeleteTextureCE(GlTextureId texture) noexcept;
    KLGL_OGL_INLINE static void DeleteTexture(GlTextureId texture);

    /************************************************** Shaders *******************************************************/

    [[nodiscard]] KLGL_OGL_INLINE static GlShaderId CreateShaderNE(GlShaderType type) noexcept;
    [[nodiscard]] KLGL_OGL_INLINE static std::expected<GlShaderId, OpenGlError> CreateShaderCE(
        GlShaderType type) noexcept;
    [[nodiscard]] KLGL_OGL_INLINE static GlShaderId CreateShader(GlShaderType type);

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

    KLGL_OGL_INLINE static void DeleteShaderNE(GlShaderId shader) noexcept;
    [[nodiscard]] KLGL_OGL_INLINE static std::optional<OpenGlError> DeleteShaderCE(GlShaderId shader) noexcept;
    KLGL_OGL_INLINE static void DeleteShader(GlShaderId shader);

    /************************************************** Program *******************************************************/

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

    [[nodiscard]] KLGL_OGL_INLINE static int32_t GetProgramIntParameterNE(
        GlProgramId program,
        GlProgramIntParameter parameter) noexcept;
    [[nodiscard]] KLGL_OGL_INLINE static std::expected<int32_t, OpenGlError> GetProgramIntParameterCE(
        GlProgramId program,
        GlProgramIntParameter parameter) noexcept;
    [[nodiscard]] KLGL_OGL_INLINE static int32_t GetProgramIntParameter(
        GlProgramId program,
        GlProgramIntParameter parameter);

    [[nodiscard]] KLGL_OGL_INLINE static size_t GetProgramActiveAttributesCountNE(GlProgramId program) noexcept;
    [[nodiscard]] KLGL_OGL_INLINE static std::expected<size_t, OpenGlError> GetProgramActiveAttributesCountCE(
        GlProgramId program) noexcept;
    [[nodiscard]] KLGL_OGL_INLINE static size_t GetProgramActiveAttributesCount(GlProgramId program);

    [[nodiscard]] KLGL_OGL_INLINE static size_t GetProgramActiveAttributeMaxNameLengthNE(GlProgramId program) noexcept;
    [[nodiscard]] KLGL_OGL_INLINE static std::expected<size_t, OpenGlError> GetProgramActiveAttributeMaxNameLengthCE(
        GlProgramId program) noexcept;
    [[nodiscard]] KLGL_OGL_INLINE static size_t GetProgramActiveAttributeMaxNameLength(GlProgramId program);

    KLGL_OGL_INLINE static void GetActiveAttributeNE(
        GlProgramId program,
        size_t attribute_index,
        size_t name_buffer_size,
        size_t& out_written_to_name_buffer,
        size_t& out_attribute_size,
        GlVertexAttributeType& out_attribute_type,
        char* out_name_buffer) noexcept;
    [[nodiscard]] KLGL_OGL_INLINE static std::optional<OpenGlError> GetActiveAttributeCE(
        GlProgramId program,
        size_t attribute_index,
        size_t name_buffer_size,
        size_t& out_written_to_name_buffer,
        size_t& out_attribute_size,
        GlVertexAttributeType& out_attribute_type,
        char* out_name_buffer) noexcept;
    KLGL_OGL_INLINE static void GetActiveAttribute(
        GlProgramId program,
        size_t attribute_index,
        size_t name_buffer_size,
        size_t& out_written_to_name_buffer,
        size_t& out_attribute_size,
        GlVertexAttributeType& out_attribute_type,
        char* out_name_buffer) noexcept;

    [[nodiscard]] KLGL_OGL_INLINE static int32_t GetAttributeLocationNE(
        GlProgramId program,
        std::string_view attribute_name) noexcept;
    [[nodiscard]] KLGL_OGL_INLINE static std::expected<size_t, OpenGlError> GetAttributeLocationCE(
        GlProgramId program,
        std::string_view attribute_name) noexcept;
    [[nodiscard]] KLGL_OGL_INLINE static size_t GetAttributeLocation(
        GlProgramId program,
        std::string_view attribute_name);

    [[nodiscard]] KLGL_OGL_INLINE static size_t GetProgramActiveUniformsCountNE(GlProgramId program) noexcept;
    [[nodiscard]] KLGL_OGL_INLINE static std::expected<size_t, OpenGlError> GetProgramActiveUniformsCountCE(
        GlProgramId program) noexcept;
    [[nodiscard]] KLGL_OGL_INLINE static size_t GetProgramActiveUniformsCount(GlProgramId program);

    [[nodiscard]] KLGL_OGL_INLINE static size_t GetProgramActiveUniformMaxNameLengthNE(GlProgramId program) noexcept;
    [[nodiscard]] KLGL_OGL_INLINE static std::expected<size_t, OpenGlError> GetProgramActiveUniformMaxNameLengthCE(
        GlProgramId program) noexcept;
    [[nodiscard]] KLGL_OGL_INLINE static size_t GetProgramActiveUniformMaxNameLength(GlProgramId program);

    KLGL_OGL_INLINE static void GetActiveUniformNE(
        GlProgramId program,
        size_t uniform_index,
        size_t name_buffer_size,
        size_t& out_written_to_name_buffer,
        size_t& out_uniform_size,
        GlUniformType& out_uniform_type,
        char* out_name_buffer) noexcept;
    [[nodiscard]] KLGL_OGL_INLINE static std::optional<OpenGlError> GetActiveUniformCE(
        GlProgramId program,
        size_t uniform_index,
        size_t name_buffer_size,
        size_t& out_written_to_name_buffer,
        size_t& out_uniform_size,
        GlUniformType& out_uniform_type,
        char* out_name_buffer) noexcept;
    KLGL_OGL_INLINE static void GetActiveUniform(
        GlProgramId program,
        size_t uniform_index,
        size_t name_buffer_size,
        size_t& out_written_to_name_buffer,
        size_t& out_uniform_size,
        GlUniformType& out_uniform_type,
        char* out_name_buffer) noexcept;

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

    [[nodiscard]] KLGL_OGL_INLINE static GLint GetUniformLocationNE(GlProgramId program, const char* name) noexcept;
    [[nodiscard]] KLGL_OGL_INLINE static std::expected<GLint, OpenGlError> GetUniformLocationCE(
        GlProgramId program,
        const char* name) noexcept;
    [[nodiscard]] KLGL_OGL_INLINE static GLint GetUniformLocation(GlProgramId program, const char* name);

    KLGL_OGL_INLINE static void SetUniformNE(uint32_t location, int32_t v) noexcept;
    [[nodiscard]] KLGL_OGL_INLINE static std::optional<OpenGlError> SetUniformCE(uint32_t location, int32_t v) noexcept;
    KLGL_OGL_INLINE static void SetUniform(uint32_t location, int32_t v);

    KLGL_OGL_INLINE static void SetUniformNE(uint32_t location, uint32_t v) noexcept;
    [[nodiscard]] KLGL_OGL_INLINE static std::optional<OpenGlError> SetUniformCE(
        uint32_t location,
        uint32_t v) noexcept;
    KLGL_OGL_INLINE static void SetUniform(uint32_t location, uint32_t v);

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

    KLGL_OGL_INLINE static void DeleteProgramNE(GlProgramId program) noexcept;
    [[nodiscard]] KLGL_OGL_INLINE static std::optional<OpenGlError> DeleteProgramCE(GlProgramId program) noexcept;
    KLGL_OGL_INLINE static void DeleteProgram(GlProgramId program);

    /*************************************************** Clear ********************************************************/

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

    /************************************************ Face Culling ****************************************************/

    KLGL_OGL_INLINE static void EnableFaceCullingNE(bool value) noexcept;
    [[nodiscard]] KLGL_OGL_INLINE static std::optional<OpenGlError> EnableFaceCullingCE(bool value) noexcept;
    KLGL_OGL_INLINE static void EnableFaceCulling(bool value);

    KLGL_OGL_INLINE static void CullFaceNE(GlCullFaceMode mode) noexcept;
    [[nodiscard]] KLGL_OGL_INLINE static std::optional<OpenGlError> CullFaceCE(GlCullFaceMode mode) noexcept;
    KLGL_OGL_INLINE static void CullFace(GlCullFaceMode mode);

    /****************************************** Vertex Attribute Pointer **********************************************/

    KLGL_OGL_INLINE static void VertexAttribPointerNE(
        size_t index,
        size_t size,
        GlVertexAttribComponentType type,
        bool normalized,
        size_t stride,
        const void* pointer) noexcept;
    KLGL_OGL_INLINE static std::optional<OpenGlError> VertexAttribPointerCE(
        size_t index,
        size_t size,
        GlVertexAttribComponentType type,
        bool normalized,
        size_t stride,
        const void* pointer) noexcept;
    KLGL_OGL_INLINE static void VertexAttribPointer(
        size_t index,
        size_t size,
        GlVertexAttribComponentType type,
        bool normalized,
        size_t stride,
        const void* pointer);

    // This function binds integer buffers as is, without converting them to floats
    KLGL_OGL_INLINE static void VertexAttribIPointerNE(
        size_t index,
        size_t size,
        GlVertexAttribComponentType type,
        size_t stride,
        const void* pointer) noexcept;
    KLGL_OGL_INLINE static std::optional<OpenGlError> VertexAttribIPointerCE(
        size_t index,
        size_t size,
        GlVertexAttribComponentType type,
        size_t stride,
        const void* pointer) noexcept;
    KLGL_OGL_INLINE static void VertexAttribIPointer(
        size_t index,
        size_t size,
        GlVertexAttribComponentType type,
        size_t stride,
        const void* pointer);

    /****************************************************** Draw ******************************************************/

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

    // Render primitives from array data
    // mode: specifies what kind of primitives to render.
    // first index: the starting index in the enabled arrays.
    // indices count: the number of indices to be rendered.
    KLGL_OGL_INLINE static void DrawArraysNE(GlPrimitiveType mode, size_t first_index, size_t indices_count) noexcept;
    [[nodiscard]] KLGL_OGL_INLINE static std::optional<OpenGlError>
    DrawArraysCE(GlPrimitiveType mode, size_t first_index, size_t indices_count) noexcept;
    KLGL_OGL_INLINE static void DrawArrays(GlPrimitiveType mode, size_t first_index, size_t indices_count);

    // Draw multiple instances of a range of elements
    // mode: specifies what kind of primitives to render.
    // first index: the starting index in the enabled arrays.
    // indices count: the number of indices to be rendered.
    // instances count: the number of instances of the specified range of indices to be rendered.
    KLGL_OGL_INLINE static void DrawArraysInstancedNE(
        GlPrimitiveType mode,
        size_t first_index,
        size_t indices_count,
        size_t instances_count) noexcept;
    [[nodiscard]] KLGL_OGL_INLINE static std::optional<OpenGlError> DrawArraysInstancedCE(
        GlPrimitiveType mode,
        size_t first_index,
        size_t indices_count,
        size_t instances_count) noexcept;
    KLGL_OGL_INLINE static void
    DrawArraysInstanced(GlPrimitiveType mode, size_t first_index, size_t indices_count, size_t instances_count);

    /******************************************************************************************************************/

    KLGL_OGL_INLINE static void EnableVertexAttribArrayNE(size_t index) noexcept;
    [[nodiscard]] KLGL_OGL_INLINE static std::optional<OpenGlError> EnableVertexAttribArrayCE(size_t index) noexcept;
    KLGL_OGL_INLINE static void EnableVertexAttribArray(size_t index);

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

    KLGL_OGL_INLINE static void GenerateMipmapNE(GLenum target) noexcept;
    KLGL_OGL_INLINE static void GenerateMipmap(GLenum target);

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
