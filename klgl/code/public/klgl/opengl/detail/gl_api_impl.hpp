#include "fmt/format.h"
#include "fmt/ranges.h"  // IWYU pragma: keep
#include "klgl/opengl/debug/annotations.hpp"
#include "klgl/opengl/detail/maps/gl_value_to_gl_error.hpp"
#include "klgl/opengl/detail/maps/to_gl_value/buffer_type.hpp"
#include "klgl/opengl/detail/maps/to_gl_value/depth_texture_compare_function.hpp"
#include "klgl/opengl/detail/maps/to_gl_value/depth_texture_compare_mode.hpp"
#include "klgl/opengl/detail/maps/to_gl_value/index_buffer_element_type.hpp"
#include "klgl/opengl/detail/maps/to_gl_value/polygon_mode.hpp"
#include "klgl/opengl/detail/maps/to_gl_value/primitive_type.hpp"
#include "klgl/opengl/detail/maps/to_gl_value/shader_type.hpp"
#include "klgl/opengl/detail/maps/to_gl_value/texture_filter.hpp"
#include "klgl/opengl/detail/maps/to_gl_value/texture_parameter_target.hpp"
#include "klgl/opengl/detail/maps/to_gl_value/texture_wrap_axis.hpp"
#include "klgl/opengl/detail/maps/to_gl_value/texture_wrap_mode.hpp"
#include "klgl/opengl/detail/maps/to_gl_value/usage.hpp"
#include "klgl/opengl/detail/maps/to_gl_value/vertex_attrib_component_type.hpp"
#include "klgl/opengl/gl_api.hpp"
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

    static void ThrowIfError(std::optional<OpenGlError> error)
    {
        [[unlikely]] if (error.has_value())
        {
            throw std::move(error.value());
        }
    }

    template <typename T>
    [[nodiscard]] static T TryTakeValue(std::expected<T, OpenGlError> expected)
    {
        if (expected.has_value())
        {
            return std::move(expected.value());
        }

        throw std::move(expected.error());
    }

    template <typename... Args>
    [[nodiscard]] static std::optional<OpenGlError> ConsumeError(fmt::format_string<Args...> format, Args&&... args)
    {
        if (GlError e = OpenGl::GetError(); e != GlError::NoError)
        {
            return OpenGlError(e, fmt::format(format, std::forward<Args>(args)...), cpptrace::generate_raw_trace(1));
        }

        return std::nullopt;
    }

    template <typename T, typename... Args>
    [[nodiscard]] static std::expected<std::decay_t<T>, OpenGlError>
    ValueOrError(T&& value, fmt::format_string<Args...> format, Args&&... args)
    {
        if (GlError e = OpenGl::GetError(); e != GlError::NoError)
        {
            return std::unexpected{
                OpenGlError(e, fmt::format(format, std::forward<Args>(args)...), cpptrace::generate_raw_trace(1))};
        }

        return std::forward<T>(value);
    }

    template <typename ValueType, typename F>
    [[nodiscard]] static auto ChainIfValue(std::expected<ValueType, OpenGlError> expected, F and_then)
        -> std::remove_cvref_t<typename std::invoke_result_t<F, ValueType>>
    {
        if (expected.has_value())
        {
            return and_then(std::move(expected.value()));
        }

        return std::unexpected{std::move(expected.error())};
    }

    template <typename T>
    static void GenObjects(T api_fn, const std::span<GLuint>& objects)
    {
        api_fn(static_cast<GLsizei>(objects.size()), objects.data());
    }

    template <typename T>
    [[nodiscard]] static GLuint GenObject(T api_fn)
    {
        GLuint object{};
        GenObjects(api_fn, std::span(&object, 1));
        return object;
    }
};

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
    return Internal::GenObject(glGenVertexArrays);
}

void OpenGl::GenVertexArrays(const std::span<GLuint>& arrays) noexcept
{
    Internal::GenObjects(glGenVertexArrays, arrays);
}

GLuint OpenGl::GenBuffer() noexcept
{
    return Internal::GenObject(glGenBuffers);
}

void OpenGl::GenBuffers(const std::span<GLuint>& buffers) noexcept
{
    return Internal::GenObjects(glGenBuffers, buffers);
}

GLuint OpenGl::GenTexture() noexcept
{
    return Internal::GenObject(glGenTextures);
}

void OpenGl::GenTextures(const std::span<GLuint>& textures) noexcept
{
    Internal::GenObjects(glGenTextures, textures);
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

GlShaderId OpenGl::CreateShaderNE(GlShaderType type) noexcept
{
    return GlShaderId::FromValue(glCreateShader(ToGlValue(type)));
}

std::expected<GlShaderId, OpenGlError> OpenGl::CreateShaderCE(GlShaderType type) noexcept
{
    return Internal::ValueOrError(CreateShaderNE(type), "glCreateShader(type: {})", type);
}

GlShaderId OpenGl::CreateShader(GlShaderType type)
{
    return Internal::TryTakeValue(CreateShaderCE(type));
}

void OpenGl::DeleteShaderNE(GlShaderId shader) noexcept
{
    glDeleteShader(shader.GetValue());
}

std::optional<OpenGlError> OpenGl::DeleteShaderCE(GlShaderId shader) noexcept
{
    DeleteShaderNE(shader);
    return Internal::ConsumeError("glDeleteShader(shader: {})", shader.GetValue());
}

void OpenGl::DeleteShader(GlShaderId shader)
{
    Internal::ThrowIfError(DeleteShaderCE(shader));
}

template <size_t stack_reserve>
struct ShaderSourceCollector
{
    std::vector<const char*> shader_sources_heap{};
    std::vector<GLint> shader_sources_lengths_heap{};
    std::array<const char*, stack_reserve> shader_sources_stack{};
    std::array<GLint, stack_reserve> shader_sources_lengths_stack{};

    std::pair<std::span<const char*>, std::span<GLint>> Fill(std::span<const std::string_view> sources)
    {
        std::span<const char*> shader_sources;
        std::span<GLint> shader_sources_lengths{};
        const size_t num_sources = sources.size();
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

        return {shader_sources, shader_sources_lengths};
    }
};

void OpenGl::ShaderSourceNE(GlShaderId shader, std::span<const std::string_view> sources) noexcept
{
    ShaderSourceCollector<30> c;
    auto [sources_span, lengths_span] = c.Fill(sources);
    glShaderSource(shader.GetValue(), static_cast<GLsizei>(sources.size()), sources_span.data(), lengths_span.data());
}

std::optional<OpenGlError> OpenGl::ShaderSourceCE(GlShaderId shader, std::span<const std::string_view> sources) noexcept
{
    ShaderSourceCollector<30> c;
    auto [sources_span, lengths_span] = c.Fill(sources);
    glShaderSource(shader.GetValue(), static_cast<GLsizei>(sources.size()), sources_span.data(), lengths_span.data());
    return Internal::ConsumeError(
        "glShaderSource(shader: {}, count: {}, strings: [{}], lengths: [{}])",
        shader.GetValue(),
        sources.size(),
        sources_span,
        lengths_span);
}

void OpenGl::ShaderSource(GlShaderId shader, std::span<const std::string_view> sources)
{
    Internal::ThrowIfError(ShaderSourceCE(shader, sources));
}

void OpenGl::CompileShaderNE(GlShaderId shader) noexcept
{
    glCompileShader(shader.GetValue());
}

std::optional<OpenGlError> OpenGl::CompileShaderCE(GlShaderId shader) noexcept
{
    CompileShaderNE(shader);
    return Internal::ConsumeError("glCompileShader(shader: {})", shader.GetValue());
}

void OpenGl::CompileShader(GlShaderId shader)
{
    Internal::ThrowIfError(CompileShaderCE(shader));
}

bool OpenGl::GetShaderCompileStatusNE(GlShaderId shader) noexcept
{
    GLint result = GL_FALSE;
    glGetShaderiv(shader.GetValue(), GL_COMPILE_STATUS, &result);
    return result == GL_TRUE;
}

std::expected<bool, OpenGlError> OpenGl::GetShaderCompileStatusCE(GlShaderId shader) noexcept
{
    return Internal::ValueOrError(
        GetShaderCompileStatusNE(shader),
        "glGetShaderiv(shader: {}, pname: GL_COMPILE_STATUS)",
        shader.GetValue());
}

bool OpenGl::GetShaderCompileStatus(GlShaderId shader)
{
    return Internal::TryTakeValue(GetShaderCompileStatusCE(shader));
}

size_t OpenGl::GetShaderLogLengthNE(GlShaderId shader) noexcept
{
    GLint log_length{};
    glGetShaderiv(shader.GetValue(), GL_INFO_LOG_LENGTH, &log_length);
    return static_cast<size_t>(std::max(0, log_length));
}

std::expected<size_t, OpenGlError> OpenGl::GetShaderLogLengthCE(GlShaderId shader) noexcept
{
    return Internal::ValueOrError(GetShaderLogLengthNE(shader), "glGetShaderiv(shader : {}) ", shader.GetValue());
}

size_t OpenGl::GetShaderLogLength(GlShaderId shader)
{
    return Internal::TryTakeValue(GetShaderLogLengthCE(shader));
}

std::string OpenGl::GetShaderLogNE(GlShaderId shader) noexcept
{
    size_t length = GetShaderLogLengthNE(shader);
    std::string log;
    log.resize(length);

    if (length)
    {
        glGetShaderInfoLog(shader.GetValue(), static_cast<GLint>(log.size()), nullptr, log.data());
    }

    return log;
}

std::expected<std::string, OpenGlError> OpenGl::GetShaderLogCE(GlShaderId shader) noexcept
{
    return Internal::ChainIfValue(
        GetShaderLogLengthCE(shader),
        [&](size_t length) -> std::expected<std::string, OpenGlError>
        {
            std::string log;
            log.resize(length);
            glGetShaderInfoLog(shader.GetValue(), static_cast<GLint>(log.size()), nullptr, log.data());

            return Internal::ValueOrError(
                std::move(log),
                "glGetShaderInfoLog(shader: {}, bufSize: {}, length: nullptr, infoLog: {})",
                shader.GetValue(),
                log.size(),
                static_cast<const void*>(log.data()));
        });
}

std::string OpenGl::GetShaderLog(GlShaderId shader)
{
    return Internal::TryTakeValue(GetShaderLogCE(shader));
}

GlProgramId OpenGl::CreateProgramNE() noexcept
{
    return GlProgramId::FromValue(glCreateProgram());
}

std::expected<GlProgramId, OpenGlError> OpenGl::CreateProgramCE() noexcept
{
    return Internal::ValueOrError(CreateProgramNE(), "glCreateProgram()");
}

GlProgramId OpenGl::CreateProgram()
{
    return Internal::TryTakeValue(CreateProgramCE());
}

void OpenGl::AttachShaderNE(GlProgramId program, GlShaderId shader) noexcept
{
    glAttachShader(program.GetValue(), shader.GetValue());
}

std::optional<OpenGlError> OpenGl::AttachShaderCE(GlProgramId program, GlShaderId shader) noexcept
{
    AttachShaderNE(program, shader);
    return Internal::ConsumeError("glAttachShader(program: {}, shader: {})", program.GetValue(), shader.GetValue());
}

void OpenGl::AttachShader(GlProgramId program, GlShaderId shader)
{
    Internal::ThrowIfError(AttachShaderCE(program, shader));
}

void OpenGl::LinkProgramNE(GlProgramId program) noexcept
{
    glLinkProgram(program.GetValue());
}

std::optional<OpenGlError> OpenGl::LinkProgramCE(GlProgramId program) noexcept
{
    LinkProgramNE(program);
    return Internal::ConsumeError("glLinkProgram(program: {} )", program.GetValue());
}

void OpenGl::LinkProgram(GlProgramId program)
{
    Internal::ThrowIfError(LinkProgramCE(program));
}

bool OpenGl::GetProgramLinkStatusNE(GlProgramId program) noexcept
{
    GLint link_status = GL_FALSE;
    glGetProgramiv(program.GetValue(), GL_LINK_STATUS, &link_status);
    return link_status == GL_TRUE;
}

std::expected<bool, OpenGlError> OpenGl::GetProgramLinkStatusCE(GlProgramId program) noexcept
{
    return Internal::ValueOrError(
        GetProgramLinkStatusNE(program),
        "glGetProgramiv(program: {}, GL_LINK_STATUS)",
        program.GetValue());
}

bool OpenGl::GetProgramLinkStatus(GlProgramId program)
{
    return Internal::TryTakeValue(GetProgramLinkStatusCE(program));
}

size_t OpenGl::GetProgramLogLengthNE(GlProgramId program) noexcept
{
    GLint log_length{};
    glGetProgramiv(program.GetValue(), GL_INFO_LOG_LENGTH, &log_length);
    return static_cast<size_t>(log_length);
}

std::expected<size_t, OpenGlError> OpenGl::GetProgramLogLengthCE(GlProgramId program) noexcept
{
    GLint log_length{};
    glGetProgramiv(program.GetValue(), GL_INFO_LOG_LENGTH, &log_length);
    return Internal::ValueOrError(log_length, "glGetProgramiv(program: {}, GL_INFO_LOG_LENGTH)", program.GetValue());
}

size_t OpenGl::GetProgramLogLength(GlProgramId program)
{
    return Internal::TryTakeValue(GetProgramLogLengthCE(program));
}

std::string OpenGl::GetProgramLogNE(GlProgramId program) noexcept
{
    size_t length = GetProgramLogLengthNE(program);
    std::string log;
    log.resize(length);

    if (length)
    {
        glGetProgramInfoLog(program.GetValue(), static_cast<GLint>(log.size()), nullptr, log.data());
    }

    return log;
}

std::expected<std::string, OpenGlError> OpenGl::GetProgramLogCE(GlProgramId program) noexcept
{
    return Internal::ChainIfValue(
        GetProgramLogLengthCE(program),
        [&](const size_t length) -> std::expected<std::string, OpenGlError>
        {
            std::string log;
            log.resize(length);
            glGetProgramInfoLog(program.GetValue(), static_cast<GLint>(log.size()), nullptr, log.data());
            return Internal::ValueOrError(
                std::move(log),
                "glGetProgramInfoLog(program: {}, bufSize: {}, length: nullptr, infoLog: {})",
                program.GetValue(),
                log.size(),
                static_cast<const void*>(log.data()));
        });
}

std::string OpenGl::GetProgramLog(GlProgramId program)
{
    return Internal::TryTakeValue(GetProgramLogCE(program));
}

void OpenGl::UseProgramNE(GlProgramId program) noexcept
{
    glUseProgram(program.GetValue());
}

std::optional<OpenGlError> OpenGl::UseProgramCE(GlProgramId program) noexcept
{
    UseProgramNE(program);
    return Internal::ConsumeError("glUseProgram(program: {})", program.GetValue());
}

void OpenGl::UseProgram(GlProgramId program)
{
    Internal::ThrowIfError(UseProgramCE(program));
}

void OpenGl::DeleteProgramNE(GlProgramId program) noexcept
{
    glDeleteProgram(program.GetValue());
}

std::optional<OpenGlError> OpenGl::DeleteProgramCE(GlProgramId program) noexcept
{
    DeleteProgramNE(program);
    return Internal::ConsumeError("glDeleteProgram(program: {})", program.GetValue());
}

void OpenGl::DeleteProgram(GlProgramId program)
{
    Internal::ThrowIfError(DeleteProgramCE(program));
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

std::optional<OpenGlError> OpenGl::VertexAttribPointerCE(
    GLuint index,
    size_t size,
    GlVertexAttribComponentType type,
    bool normalized,
    size_t stride,
    const void* pointer) noexcept
{
    VertexAttribPointerNE(index, size, type, normalized, stride, pointer);
    return Internal::ConsumeError(
        "glVertexAttribPointer(index: {}, size: {}, type: {}, normalized: {}, stride: {}, pointer: {})",
        index,
        size,
        type,
        normalized,
        stride,
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
    Internal::ThrowIfError(VertexAttribPointerCE(index, size, type, normalized, stride, pointer));
}

void OpenGl::EnableVertexAttribArrayNE(GLuint index) noexcept
{
    glEnableVertexAttribArray(index);
}

std::optional<OpenGlError> OpenGl::EnableVertexAttribArrayCE(GLuint index) noexcept
{
    EnableVertexAttribArrayNE(index);
    return Internal::ConsumeError("glEnableVertexAttribArray(index: {})", index);
}

void OpenGl::EnableVertexAttribArray(GLuint index)
{
    Internal::ThrowIfError(EnableVertexAttribArrayCE(index));
}

void OpenGl::EnableDepthTestNE() noexcept
{
    glEnable(GL_DEPTH_TEST);
}

std::optional<OpenGlError> OpenGl::EnableDepthTestCE() noexcept
{
    EnableDepthTestNE();
    return Internal::ConsumeError("glEnable(GL_DEPTH_TEST)");
}

void OpenGl::EnableDepthTest()
{
    Internal::ThrowIfError(EnableDepthTestCE());
}

void OpenGl::EnableBlendingNE() noexcept
{
    glEnable(GL_BLEND);
}

std::optional<OpenGlError> OpenGl::EnableBlendingCE() noexcept
{
    EnableBlendingNE();
    return Internal::ConsumeError("glEnable(GL_BLEND)");
}

void OpenGl::EnableBlending()
{
    Internal::ThrowIfError(EnableBlendingCE());
}

void OpenGl::ViewportNE(GLint x, GLint y, GLsizei width, GLsizei height) noexcept
{
    glViewport(x, y, width, height);
}

std::optional<OpenGlError> OpenGl::ViewportCE(GLint x, GLint y, GLsizei width, GLsizei height) noexcept
{
    ViewportNE(x, y, width, height);
    return Internal::ConsumeError("glViewport(x: {}, y: {}, width: {}, height {})", x, y, width, height);
}

void OpenGl::Viewport(GLint x, GLint y, GLsizei width, GLsizei height)
{
    Internal::ThrowIfError(ViewportCE(x, y, width, height));
}

void OpenGl::SetClearColorNE(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha) noexcept
{
    glClearColor(red, green, blue, alpha);
}

std::optional<OpenGlError> OpenGl::SetClearColorCE(GLfloat r, GLfloat g, GLfloat b, GLfloat a) noexcept
{
    SetClearColorNE(r, g, b, a);
    return Internal::ConsumeError("glClearColor(r: {}, g: {}, b: {}, a: {})", r, g, b, a);
}

void OpenGl::SetClearColor(GLfloat r, GLfloat g, GLfloat b, GLfloat a)
{
    Internal::ThrowIfError(SetClearColorCE(r, g, b, a));
}

void OpenGl::SetClearColorNE(const Vec4f& color) noexcept
{
    SetClearColorNE(color.x(), color.y(), color.z(), color.w());
}

std::optional<OpenGlError> OpenGl::SetClearColorCE(const Vec4f& color) noexcept
{
    return SetClearColorCE(color.x(), color.y(), color.z(), color.w());
}

void OpenGl::SetClearColor(const Vec4f& color)
{
    SetClearColor(color.x(), color.y(), color.z(), color.w());
}

void OpenGl::ClearNE(GLbitfield mask) noexcept
{
    glClear(mask);
}

std::optional<OpenGlError> OpenGl::ClearCE(GLbitfield mask) noexcept
{
    ClearNE(mask);
    return Internal::ConsumeError("glClear(mask: {})", mask);
}

void OpenGl::Clear(GLbitfield mask) noexcept
{
    Internal::ThrowIfError(ClearCE(mask));
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

std::optional<OpenGlError> OpenGl::DrawElementsCE(
    GlPrimitiveType mode,
    size_t num,
    GlIndexBufferElementType indices_type,
    const void* indices) noexcept
{
    DrawElementsNE(mode, num, indices_type, indices);
    return Internal::ConsumeError(
        "glDrawElements(mode: {}, num: {}, indices_type: {}, indices: {})",
        mode,
        num,
        indices_type,
        indices);
}

void OpenGl::DrawElements(GlPrimitiveType mode, size_t num, GlIndexBufferElementType indices_type, const void* indices)
{
    Internal::ThrowIfError(DrawElementsCE(mode, num, indices_type, indices));
}

void OpenGl::DrawElementsInstancedNE(
    GlPrimitiveType mode,
    size_t num,
    GlIndexBufferElementType indices_type,
    const void* indices,
    size_t num_instances) noexcept
{
    ScopeAnnotation annotation("OpenGl::DrawInstanced");
    glDrawElementsInstanced(
        ToGlValue(mode),
        static_cast<GLsizei>(num),
        ToGlValue(indices_type),
        indices,
        static_cast<GLsizei>(num_instances));
}

std::optional<OpenGlError> OpenGl::DrawElementsInstancedCE(
    GlPrimitiveType mode,
    size_t num,
    GlIndexBufferElementType indices_type,
    const void* indices,
    size_t num_instances) noexcept
{
    DrawElementsInstancedNE(mode, num, indices_type, indices, num_instances);
    return Internal::ConsumeError(
        "glDrawElementsInstanced(mode: {}, count {}, type: {}, indices: {}, instancecount: {})",
        mode,
        num,
        indices_type,
        indices,
        num_instances);
}

void OpenGl::DrawElementsInstanced(
    GlPrimitiveType mode,
    size_t num,
    GlIndexBufferElementType indices_type,
    const void* indices,
    size_t num_instances)
{
    Internal::ThrowIfError(DrawElementsInstancedCE(mode, num, indices_type, indices, num_instances));
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

std::optional<OpenGlError> OpenGl::SetUniformCE(uint32_t location, const float& f) noexcept
{
    SetUniformNE(location, f);
    return Internal::ConsumeError("glUniform1f(location: {}, value: {})", location, f);
}

void OpenGl::SetUniform(uint32_t location, const float& f)
{
    Internal::ThrowIfError(SetUniformCE(location, f));
}

void OpenGl::SetUniformNE(uint32_t location, const Vec2f& v) noexcept
{
    glUniform2f(static_cast<GLint>(location), v.x(), v.y());
}

std::optional<OpenGlError> OpenGl::SetUniformCE(uint32_t location, const Vec2f& v) noexcept
{
    SetUniformNE(location, v);
    return Internal::ConsumeError("glUniform2f(location: {}, v: {})", location, v.data_);
}

void OpenGl::SetUniform(uint32_t location, const Vec2f& v)
{
    Internal::ThrowIfError(SetUniformCE(location, v));
}

void OpenGl::SetUniformNE(uint32_t location, const Vec3f& v) noexcept
{
    glUniform3f(static_cast<GLint>(location), v.x(), v.y(), v.z());
}

std::optional<OpenGlError> OpenGl::SetUniformCE(uint32_t location, const Vec3f& v) noexcept
{
    SetUniformNE(location, v);
    return Internal::ConsumeError("glUniform3f(location: {}, v: {})", location, v.data_);
}

void OpenGl::SetUniform(uint32_t location, const Vec3f& v)
{
    SetUniformNE(location, v);
    Internal::ThrowIfError(SetUniformCE(location, v));
}

void OpenGl::SetUniformNE(uint32_t location, const Vec4f& v) noexcept
{
    glUniform4f(static_cast<GLint>(location), v.x(), v.y(), v.z(), v.w());
}

std::optional<OpenGlError> OpenGl::SetUniformCE(uint32_t location, const Vec4f& v) noexcept
{
    SetUniformNE(location, v);
    return Internal::ConsumeError("glUniform4f(location: {}, v: {})", location, v.data_);
}

void OpenGl::SetUniform(uint32_t location, const Vec4f& v)
{
    Internal::ThrowIfError(SetUniformCE(location, v));
}

void OpenGl::SetUniformNE(uint32_t location, const Mat3f& m, bool transpose) noexcept
{
    glUniformMatrix3fv(static_cast<GLint>(location), 1, CastBool(transpose), m.data());
}

std::optional<OpenGlError> OpenGl::SetUniformCE(uint32_t location, const Mat3f& m, bool transpose) noexcept
{
    SetUniformNE(location, m, transpose);
    return Internal::ConsumeError(
        "glUniformMatrix3fv(location: {}, matrix: {}, transpose: {})",
        location,
        m.data_,
        transpose);
}

void OpenGl::SetUniform(uint32_t location, const Mat3f& m, bool transpose)
{
    Internal::ThrowIfError(SetUniformCE(location, m, transpose));
}

void OpenGl::SetUniformNE(uint32_t location, const Mat4f& m, bool transpose) noexcept
{
    glUniformMatrix4fv(static_cast<GLint>(location), 1, CastBool(transpose), m.data());
}

std::optional<OpenGlError> OpenGl::SetUniformCE(uint32_t location, const Mat4f& m, bool transpose) noexcept
{
    SetUniformNE(location, m, transpose);
    return Internal::ConsumeError(
        "glUniformMatrix4fv(location: {}, matrix: {}, transpose: {})",
        location,
        m.data_,
        transpose);
}

void OpenGl::SetUniform(uint32_t location, const Mat4f& m, bool transpose)
{
    Internal::ThrowIfError(SetUniformCE(location, m, transpose));
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

void OpenGl::SetTextureWrapNE(GlTextureParameterTarget target, GlTextureWrapAxis wrap, GlTextureWrapMode mode) noexcept
{
    glTexParameteri(ToGlValue(target), ToGlValue(wrap), ToGlValue(mode));
}

void OpenGl::SetTextureWrap(GlTextureParameterTarget target, GlTextureWrapAxis wrap, GlTextureWrapMode mode)
{
    SetTextureWrapNE(target, wrap, mode);
    Internal::Check("glTexParameteri(target: {}, axis: {}, wrap: {})", target, wrap, mode);
}

void OpenGl::SetTextureMinFilterNE(GlTextureParameterTarget target, GlTextureFilter filter) noexcept
{
    glTexParameteri(ToGlValue(target), GL_TEXTURE_MIN_FILTER, ToGlValue(filter));
}

void OpenGl::SetTextureMinFilter(GlTextureParameterTarget target, GlTextureFilter filter)
{
    SetTextureMinFilterNE(target, filter);
    Internal::Check("glTexParameteri(target: {}, GL_TEXTURE_MIN_FILTER, filter: {})", target, filter);
}

void OpenGl::SetTextureMagFilterNE(GlTextureParameterTarget target, GlTextureFilter filter) noexcept
{
    glTexParameteri(ToGlValue(target), GL_TEXTURE_MAG_FILTER, ToGlValue(filter));
}

void OpenGl::SetTextureMagFilter(GlTextureParameterTarget target, GlTextureFilter filter)
{
    SetTextureMagFilterNE(target, filter);
    Internal::Check("glTexParameteri(target: {}, GL_TEXTURE_MAG_FILTER, filter: {})", target, filter);
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
