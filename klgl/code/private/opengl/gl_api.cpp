#include "klgl/opengl/gl_api.hpp"

#include "fmt/format.h"
#include "fmt/ranges.h"  // IWYU pragma: keep
#include "klgl/opengl/debug/annotations.hpp"
#include "klgl/opengl/detail/maps/gl_buffer_type_to_gl_value.hpp"
#include "klgl/opengl/detail/maps/gl_depth_texture_compare_function_to_gl_value.hpp"
#include "klgl/opengl/detail/maps/gl_depth_texture_compare_mode_to_gl_value.hpp"
#include "klgl/opengl/detail/maps/gl_index_buffer_element_type_to_gl_value.hpp"
#include "klgl/opengl/detail/maps/gl_primitive_type_to_gl_value.hpp"
#include "klgl/opengl/detail/maps/gl_texture_parameter_target_to_gl_value.hpp"
#include "klgl/opengl/detail/maps/gl_usage_to_gl_value.hpp"
#include "klgl/opengl/detail/maps/gl_value_to_gl_error.hpp"
#include "klgl/opengl/detail/maps/gl_vertex_attrib_component_type_to_gl_value.hpp"
#include "klgl/opengl/open_gl_error.hpp"

namespace klgl
{

struct OpenGl::Internal
{
    template <typename... Args>
    static void Check(fmt::format_string<Args...> format_string, Args&&... args)
    {
        [[unlikely]] if (const auto error = OpenGl::GetError(); error != GlError::NoError)
        {
            std::string message = fmt::format("OpenGL error: {}. Context: ", error);
            fmt::format_to(std::back_inserter(message), format_string, std::forward<Args>(args)...);
            throw OpenGlError(error, std::move(message));
        }
    }
};

template <typename T>
void GenObjects(T api_fn, const std::span<GLuint>& objects)
{
    api_fn(static_cast<GLsizei>(objects.size()), objects.data());
}

template <typename T>
GLuint GenObject(T api_fn)
{
    GLuint object{};
    GenObjects(api_fn, std::span(&object, 1));
    return object;
}

GlError OpenGl::GetError() noexcept
{
    const auto error = glGetError();
    [[likely]] if (error == GL_NO_ERROR)
    {
        return GlError::NoError;
    }

    if (!kGlValueToGlError.Contains(error))
    {
        return GlError::Unknown;
    }

    return kGlValueToGlError.Get(error);
}

GLuint OpenGl::GenVertexArray() noexcept
{
    return GenObject(glGenVertexArrays);
}

void OpenGl::GenVertexArrays(const std::span<GLuint>& arrays) noexcept
{
    GenObjects(glGenVertexArrays, arrays);
}

GLuint OpenGl::GenBuffer() noexcept
{
    return GenObject(glGenBuffers);
}

void OpenGl::GenBuffers(const std::span<GLuint>& buffers) noexcept
{
    return GenObjects(glGenBuffers, buffers);
}

GLuint OpenGl::GenTexture() noexcept
{
    return GenObject(glGenTextures);
}

void OpenGl::GenTextures(const std::span<GLuint>& textures) noexcept
{
    GenObjects(glGenTextures, textures);
}

void OpenGl::BindVertexArrayNE(GLuint array) noexcept
{
    glBindVertexArray(array);
}

void OpenGl::BindVertexArray(GLuint array)
{
    BindVertexArrayNE(array);
    Internal::Check("glBindVertexArray(array: {})", array);
}

void OpenGl::BindBufferNE(GlBufferType target, GLuint buffer) noexcept
{
    glBindBuffer(ToGlValue(target), buffer);
}

void OpenGl::BindBuffer(GlBufferType target, GLuint buffer)
{
    BindBufferNE(target, buffer);
    Internal::Check("glBindBuffer(target: {}, buffer: {})", target, buffer);
}

void OpenGl::BufferDataNE(GlBufferType target, GLsizeiptr size, const void* data, GlUsage usage) noexcept
{
    glBufferData(ToGlValue(target), size, data, ToGlValue(usage));
}

void OpenGl::BufferData(GlBufferType target, GLsizeiptr size, const void* data, GlUsage usage)
{
    BufferDataNE(target, size, data, usage);
    Internal::Check("glBufferData(target: {}, size: {}, data: {}, usage: {})", target, size, data, usage);
}

constexpr GLboolean OpenGl::CastBool(bool value) noexcept
{
    return static_cast<GLboolean>(value);
}

void OpenGl::VertexAttribPointerNE(
    GLuint index,
    size_t size,
    GlVertexAttribComponentType type,
    bool normalized,
    size_t stride,
    const void* pointer) noexcept
{
    glVertexAttribPointer(
        index,
        static_cast<GLint>(size),
        ToGlValue(type),
        CastBool(normalized),
        static_cast<GLsizei>(stride),
        pointer);
}

void OpenGl::VertexAttribPointer(
    GLuint index,
    size_t size,
    GlVertexAttribComponentType type,
    bool normalized,
    size_t stride,
    const void* pointer)
{
    VertexAttribPointerNE(index, size, type, normalized, stride, pointer);
    Internal::Check(
        "glVertexAttribPointer(index: {}, size: {}, type: {}, normalized: {}, stride: {}, pointer: {})",
        index,
        size,
        type,
        normalized,
        stride,
        pointer);
}

void OpenGl::EnableVertexAttribArrayNE(GLuint index) noexcept
{
    glEnableVertexAttribArray(index);
}

void OpenGl::EnableVertexAttribArray(GLuint index)
{
    EnableVertexAttribArrayNE(index);
    Internal::Check("glEnableVertexAttribArray(index: {})", index);
}

void OpenGl::ViewportNE(GLint x, GLint y, GLsizei width, GLsizei height) noexcept
{
    glViewport(x, y, width, height);
}

void OpenGl::Viewport(GLint x, GLint y, GLsizei width, GLsizei height)
{
    ViewportNE(x, y, width, height);
    Internal::Check("glViewport(x: {}, y: {}, width: {}, height {})", x, y, width, height);
}

void OpenGl::SetClearColorNE(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha) noexcept
{
    glClearColor(red, green, blue, alpha);
}

void OpenGl::SetClearColor(GLfloat r, GLfloat g, GLfloat b, GLfloat a)
{
    SetClearColorNE(r, g, b, a);
    Internal::Check("glClearColor(r: {}, g: {}, b: {}, a: {})", r, g, b, a);
}

void OpenGl::SetClearColorNE(const Vec4f& color) noexcept
{
    SetClearColorNE(color.x(), color.y(), color.z(), color.w());
}

void OpenGl::SetClearColor(const Vec4f& color)
{
    SetClearColor(color.x(), color.y(), color.z(), color.w());
}

void OpenGl::ClearNE(GLbitfield mask) noexcept
{
    glClear(mask);
}

void OpenGl::Clear(GLbitfield mask) noexcept
{
    ClearNE(mask);
    Internal::Check("glClear(mask: {})", mask);
}

void OpenGl::UseProgramNE(GLuint program) noexcept
{
    glUseProgram(program);
}

void OpenGl::UseProgram(GLuint program)
{
    UseProgramNE(program);
    Internal::Check("glUseProgram(program: {})", program);
}

void OpenGl::DrawElementsNE(
    GlPrimitiveType mode,
    size_t num,
    GlIndexBufferElementType indices_type,
    const void* indices) noexcept
{
    ScopeAnnotation annotation("OpenGl::Draw");
    glDrawElements(ToGlValue(mode), static_cast<GLsizei>(num), ToGlValue(indices_type), indices);
}

void OpenGl::DrawElements(GlPrimitiveType mode, size_t num, GlIndexBufferElementType indices_type, const void* indices)
{
    DrawElementsNE(mode, num, indices_type, indices);
    Internal::Check(
        "glDrawElements(mode: {}, num: {}, indices_type: {}, indices: {})",
        mode,
        num,
        indices_type,
        indices);
}

void OpenGl::DrawElementsInstancedNE(
    GlPrimitiveType mode,
    size_t num,
    GlIndexBufferElementType indices_type,
    const void* indices,
    size_t num_instances) noexcept
{
    glDrawElementsInstanced(
        ToGlValue(mode),
        static_cast<GLsizei>(num),
        ToGlValue(indices_type),
        indices,
        static_cast<GLsizei>(num_instances));
}

void OpenGl::DrawElementsInstanced(
    GlPrimitiveType mode,
    size_t num,
    GlIndexBufferElementType indices_type,
    const void* indices,
    size_t num_instances)
{
    DrawElementsInstancedNE(mode, num, indices_type, indices, num_instances);
    Internal::Check(
        "glDrawElementsInstanced(mode: {}, count {}, type: {}, indices: {}, instancecount: {})",
        mode,
        num,
        indices_type,
        indices,
        num_instances);
}

std::optional<uint32_t> OpenGl::FindUniformLocationNE(GLuint shader_program, const char* name) noexcept
{
    int result = glGetUniformLocation(shader_program, name);
    [[likely]] if (result >= 0)
    {
        return static_cast<uint32_t>(result);
    }
    return std::optional<uint32_t>();
}

std::optional<uint32_t> OpenGl::FindUniformLocation(GLuint shader_program, const char* name)
{
    auto result = FindUniformLocationNE(shader_program, name);
    Internal::Check("glGetUniformLocation(program: {}, name: {})", shader_program, name);
    return result;
}

uint32_t OpenGl::GetUniformLocation(GLuint shader_program, const char* name)
{
    auto location = FindUniformLocation(shader_program, name);
    [[likely]] if (location.has_value())
    {
        return *location;
    }

    throw cpptrace::invalid_argument(fmt::format("Uniform with name {} was not found", name));
}

void OpenGl::SetUniformNE(uint32_t location, const float& f) noexcept
{
    glUniform1f(static_cast<GLint>(location), f);
}

void OpenGl::SetUniformNE(uint32_t location, const Mat4f& m, bool transpose) noexcept
{
    glUniformMatrix4fv(static_cast<GLint>(location), 1, CastBool(transpose), m.data());
}

void OpenGl::SetUniformNE(uint32_t location, const Mat3f& m, bool transpose) noexcept
{
    glUniformMatrix3fv(static_cast<GLint>(location), 1, CastBool(transpose), m.data());
}

void OpenGl::SetUniformNE(uint32_t location, const Vec4f& v) noexcept
{
    glUniform4f(static_cast<GLint>(location), v.x(), v.y(), v.z(), v.w());
}

void OpenGl::SetUniformNE(uint32_t location, const Vec3f& v) noexcept
{
    glUniform3f(static_cast<GLint>(location), v.x(), v.y(), v.z());
}

void OpenGl::SetUniformNE(uint32_t location, const Vec2f& v) noexcept
{
    glUniform2f(static_cast<GLint>(location), v.x(), v.y());
}

void OpenGl::SetUniform(uint32_t location, const float& f)
{
    SetUniformNE(location, f);
    Internal::Check("glUniform1f(location: {}, value: {})", location, f);
}

void OpenGl::SetUniform(uint32_t location, const Mat4f& m, bool transpose)
{
    SetUniformNE(location, m, transpose);
    Internal::Check("glUniformMatrix4fv(location: {}, matrix: {}, transpose: {})", location, m.data_, transpose);
}

void OpenGl::SetUniform(uint32_t location, const Mat3f& m, bool transpose)
{
    SetUniformNE(location, m, transpose);
    Internal::Check("glUniformMatrix3fv(location: {}, matrix: {}, transpose: {})", location, m.data_, transpose);
}

void OpenGl::SetUniform(uint32_t location, const Vec4f& v)
{
    SetUniformNE(location, v);
    Internal::Check("glUniform4f(location: {}, v: {})", location, v.data_);
}

void OpenGl::SetUniform(uint32_t location, const Vec3f& v)
{
    SetUniformNE(location, v);
    Internal::Check("glUniform3f(location: {}, v: {})", location, v.data_);
}

void OpenGl::SetUniform(uint32_t location, const Vec2f& v)
{
    SetUniformNE(location, v);
    Internal::Check("glUniform2f(location: {}, v: {})", location, v.data_);
}

void OpenGl::EnableDepthTestNE() noexcept
{
    glEnable(GL_DEPTH_TEST);
}

void OpenGl::EnableDepthTest()
{
    EnableDepthTestNE();
    Internal::Check("glEnable(GL_DEPTH_TEST)");
}

void OpenGl::EnableBlendingNE() noexcept
{
    glEnable(GL_BLEND);
}

void OpenGl::EnableBlending()
{
    EnableBlendingNE();
    Internal::Check("glEnable(GL_BLEND)");
}

void OpenGl::SetDepthTextureCompareModeNE(GlTextureParameterTarget target, GlDepthTextureCompareMode mode) noexcept
{
    glTextureParameteri(ToGlValue(target), GL_TEXTURE_COMPARE_MODE, ToGlValue(mode));
}

void OpenGl::SetDepthTextureCompareMode(GlTextureParameterTarget target, GlDepthTextureCompareMode mode)
{
    SetDepthTextureCompareModeNE(target, mode);
    Internal::Check("glTexParameteri(target: {}, name: GL_TEXTURE_COMPARE_MODE, value: {})", target, mode);
}

void OpenGl::SetTextureParameterNE(GlTextureParameterTarget target, GLenum pname, const GLfloat* value) noexcept
{
    glTexParameterfv(ToGlValue(target), pname, value);
}

void OpenGl::SetDepthTextureCompareFunctionNE(
    GlTextureParameterTarget target,
    GlDepthTextureCompareFunction function) noexcept
{
    glTexParameteri(ToGlValue(target), GL_TEXTURE_COMPARE_FUNC, ToGlValue(function));
}

void OpenGl::SetDepthTextureCompareFunction(GlTextureParameterTarget target, GlDepthTextureCompareFunction function)
{
    SetDepthTextureCompareFunctionNE(target, function);
    Internal::Check("glTexParameteri(target: {}, pname: GL_TEXTURE_COMPARE_FUNCT, param: {})", target, function);
}

void OpenGl::SetTextureBaseLevelNE(GlTextureParameterTarget target, size_t level) noexcept
{
    glTexParameteri(ToGlValue(target), GL_TEXTURE_BASE_LEVEL, static_cast<GLint>(level));
}

void OpenGl::SetTextureBaseLevel(GlTextureParameterTarget target, size_t level)
{
    SetTextureBaseLevelNE(target, level);
    Internal::Check("glTexParameteri(target: {}, pname: GL_TEXTURE_BASE_LEVEL, param: {})", target, level);
}

void OpenGl::SetTextureBorderColorNE(
    GlTextureParameterTarget target,
    std::span<const GLint, 4> color,
    bool store_as_integer) noexcept
{
    if (store_as_integer)
    {
        glTexParameterIiv(ToGlValue(target), GL_TEXTURE_BORDER_COLOR, color.data());
    }
    else
    {
        glTexParameteriv(ToGlValue(target), GL_TEXTURE_BORDER_COLOR, color.data());
    }
}

void OpenGl::SetTextureBorderColor(
    GlTextureParameterTarget target,
    std::span<const GLint, 4> color,
    bool store_as_integer)
{
    SetTextureBorderColorNE(target, color, store_as_integer);
    if (store_as_integer)
    {
        Internal::Check("glTexParameterIiv(target: {}, pname: GL_TEXTURE_BORDER_COLOR, color: {})", target, color);
    }
    else
    {
        Internal::Check("glTexParameteriv(target: {}, pname: GL_TEXTURE_BORDER_COLOR, color: {})", target, color);
    }
}

void OpenGl::SetTextureBorderColorNE(GlTextureParameterTarget target, std::span<const GLuint, 4> color) noexcept
{
    glTexParameterIuiv(ToGlValue(target), GL_TEXTURE_BORDER_COLOR, color.data());
}

void OpenGl::SetTextureBorderColor(GlTextureParameterTarget target, std::span<const GLuint, 4> color)
{
    SetTextureBorderColorNE(target, color);
    Internal::Check("glTexParameterIuiv(target: {}, pname: GL_TEXTURE_BORDER_COLOR, color: {})", target, color);
}

void OpenGl::SetTextureBorderColorNE(GlTextureParameterTarget target, std::span<const GLfloat, 4> color) noexcept
{
    glTexParameterfv(ToGlValue(target), GL_TEXTURE_BORDER_COLOR, color.data());
}

void OpenGl::SetTextureBorderColor(GlTextureParameterTarget target, std::span<const GLfloat, 4> color)
{
    SetTextureBorderColorNE(target, color);
    Internal::Check("glTexParameterfv(target: {}, pname: GL_TEXTURE_BORDER_COLOR, color: {})", target, color);
}

void OpenGl::SetTextureLODBiasNE(GlTextureParameterTarget target, float bias) noexcept
{
    glTexParameterf(ToGlValue(target), GL_TEXTURE_LOD_BIAS, bias);
}

void OpenGl::SetTextureLODBias(GlTextureParameterTarget target, float bias)
{
    SetTextureLODBiasNE(target, bias);
    Internal::Check("glTexParameterf(target: {}, pname: GL_TEXTURE_LOD_BIAS, bias: {})", target, bias);
}

void OpenGl::SetTextureParameter(GlTextureParameterTarget target, GLenum pname, const GLfloat* value)
{
    SetTextureParameterNE(target, pname, value);
    Internal::Check(
        "glTexParameterfv(target: {}, name: {}, values_ptr: {})",
        target,
        pname,
        static_cast<const void*>(value));
}

void OpenGl::SetTextureParameterNE(GlTextureParameterTarget target, GLenum name, GLint param) noexcept
{
    glTexParameteri(ToGlValue(target), name, param);
}

void OpenGl::SetTextureParameter(GlTextureParameterTarget target, GLenum name, GLint param)
{
    SetTextureParameterNE(target, name, param);
    Internal::Check("glTexParameteri(target: {}, name: {}, param: {})", target, name, param);
}

void OpenGl::SetTexture2dBorderColorNE(const Vec4f& v) noexcept
{
    SetTextureParameter2dNE(GL_TEXTURE_BORDER_COLOR, v.data());
}

void OpenGl::SetTexture2dBorderColor(const Vec4f& v)
{
    SetTexture2dBorderColorNE(v);
}

void OpenGl::SetTexture2dWrapNE(GlTextureWrapAxis wrap, GlTextureWrapMode mode) noexcept
{
    SetTextureParameter2dNE(ToGlValue(wrap), ToGlValue(mode));
}

void OpenGl::SetTexture2dWrap(GlTextureWrapAxis wrap, GlTextureWrapMode mode)
{
    SetTexture2dWrapNE(wrap, mode);
}

void OpenGl::SetTexture2dMinFilterNE(GlTextureFilter filter) noexcept
{
    SetTextureParameter2dNE(GL_TEXTURE_MIN_FILTER, ToGlValue(filter));
}

void OpenGl::SetTexture2dMinFilter(GlTextureFilter filter)
{
    SetTexture2dMinFilterNE(filter);
}

void OpenGl::SetTexture2dMagFilterNE(GlTextureFilter filter) noexcept
{
    SetTextureParameter2dNE(GL_TEXTURE_MAG_FILTER, ToGlValue(filter));
}

void OpenGl::SetTexture2dMagFilter(GlTextureFilter filter)
{
    SetTexture2dMagFilterNE(filter);
}

void OpenGl::BindTextureNE(GLenum target, GLuint texture) noexcept
{
    glBindTexture(target, texture);
}

void OpenGl::BindTexture(GLenum target, GLuint texture)
{
    BindTextureNE(target, texture);
    Internal::Check("glBindTexture(target: {}, texture: {})", target, texture);
}

void OpenGl::BindTexture2d(GLuint texture)
{
    BindTexture(GL_TEXTURE_2D, texture);
}

void OpenGl::TexImage2dNE(
    GLenum target,
    size_t level_of_detail,
    GLint internal_format,
    size_t width,
    size_t height,
    GLint data_format,
    GLenum pixel_data_type,
    const void* pixels) noexcept
{
    glTexImage2D(
        target,
        static_cast<GLint>(level_of_detail),
        internal_format,
        static_cast<GLsizei>(width),
        static_cast<GLsizei>(height),
        0,
        data_format,
        pixel_data_type,
        pixels);
}

void OpenGl::TexImage2d(
    GLenum target,
    size_t level_of_detail,
    GLint internal_format,
    size_t width,
    size_t height,
    GLint data_format,
    GLenum pixel_data_type,
    const void* pixels)
{
    TexImage2dNE(target, level_of_detail, internal_format, width, height, data_format, pixel_data_type, pixels);
    Internal::Check(
        "glTexImage2D(target: {}, lod: {}, internal_format: {}, width: {}, height: {}, pixel_buffer_layout: {}, "
        "pixel_buffer_type: {}, pixels: {})",
        target,
        level_of_detail,
        internal_format,
        width,
        height,
        data_format,
        pixel_data_type,
        pixels);
}

void OpenGl::GenerateMipmapNE(GLenum target) noexcept
{
    glGenerateMipmap(target);
}

void OpenGl::GenerateMipmap(GLenum target)
{
    GenerateMipmapNE(target);
    Internal::Check("glGenerateMipmap(target: {})", target);
}

void OpenGl::GenerateMipmap2dNE() noexcept
{
    GenerateMipmap(GL_TEXTURE_2D);
}

void OpenGl::GenerateMipmap2d()
{
    GenerateMipmap2dNE();
}

void OpenGl::PolygonModeNE(GlPolygonMode mode) noexcept
{
    glPolygonMode(GL_FRONT_AND_BACK, ToGlValue(mode));
}

void OpenGl::PolygonMode(GlPolygonMode mode)
{
    PolygonModeNE(mode);
    Internal::Check("glPolygonMode(face: GL_FRONT_AND_BACK, mode: {})", mode);
}

void OpenGl::PointSizeNE(float size) noexcept
{
    glPointSize(size);
}

void OpenGl::PointSize(float size)
{
    PointSizeNE(size);
    Internal::Check("glPointSize(size: {})", size);
}

void OpenGl::LineWidthNE(float width) noexcept
{
    glLineWidth(width);
}

void OpenGl::LineWidth(float width)
{
    LineWidthNE(width);
    Internal::Check("glLineWidth(width: {})", width);
}

}  // namespace klgl
