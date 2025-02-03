#pragma once

#include "fmt/format.h"
#include "fmt/ranges.h"  // IWYU pragma: keep
#include "identifiers_impl.hpp"
#include "klgl/opengl/debug/annotations.hpp"
#include "klgl/opengl/detail/maps/gl_value_to_gl_error.hpp"
#include "klgl/opengl/detail/maps/to_gl_value/buffer_type.hpp"
#include "klgl/opengl/detail/maps/to_gl_value/cull_face_mode.hpp"
#include "klgl/opengl/detail/maps/to_gl_value/depth_texture_compare_function.hpp"
#include "klgl/opengl/detail/maps/to_gl_value/depth_texture_compare_mode.hpp"
#include "klgl/opengl/detail/maps/to_gl_value/framebuffer_attachment.hpp"
#include "klgl/opengl/detail/maps/to_gl_value/framebuffer_bind_target.hpp"
#include "klgl/opengl/detail/maps/to_gl_value/index_buffer_element_type.hpp"
#include "klgl/opengl/detail/maps/to_gl_value/polygon_mode.hpp"
#include "klgl/opengl/detail/maps/to_gl_value/primitive_type.hpp"
#include "klgl/opengl/detail/maps/to_gl_value/program_int_parameter.hpp"
#include "klgl/opengl/detail/maps/to_gl_value/shader_type.hpp"
#include "klgl/opengl/detail/maps/to_gl_value/target_texture_type.hpp"
#include "klgl/opengl/detail/maps/to_gl_value/texture_filter.hpp"
#include "klgl/opengl/detail/maps/to_gl_value/texture_internal_format.hpp"
#include "klgl/opengl/detail/maps/to_gl_value/texture_param_type.hpp"
#include "klgl/opengl/detail/maps/to_gl_value/texture_wrap_axis.hpp"
#include "klgl/opengl/detail/maps/to_gl_value/texture_wrap_mode.hpp"
#include "klgl/opengl/detail/maps/to_gl_value/uniform_type.hpp"
#include "klgl/opengl/detail/maps/to_gl_value/usage.hpp"
#include "klgl/opengl/detail/maps/to_gl_value/vertex_attrib_component_type.hpp"
#include "klgl/opengl/detail/maps/to_gl_value/vertex_attribute_type.hpp"
#include "klgl/opengl/gl_api.hpp"
#include "klgl/opengl/open_gl_error.hpp"

namespace klgl
{

struct OpenGl::Internal
{
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
                shader_sources = std::span{shader_sources_stack}.subspan(0, num_sources);
                shader_sources_lengths = std::span{shader_sources_lengths_stack}.subspan(0, num_sources);
            }

            for (size_t i = 0; i < sources.size(); ++i)
            {
                shader_sources[i] = sources[i].data();
                shader_sources_lengths[i] = static_cast<GLsizei>(sources[i].size());
            }

            return {shader_sources, shader_sources_lengths};
        }
    };

    [[nodiscard]] static constexpr GLboolean CastBool(bool value) noexcept { return static_cast<GLboolean>(value); }

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
    [[nodiscard]] static T TryTakeValue(tl::expected<T, OpenGlError> expected)
    {
        if (expected.has_value())
        {
            return std::move(expected.value());
        }

        throw std::move(expected.error());
    }

    [[nodiscard]] inline static tl::expected<size_t, OpenGlError> CastToSizeT(int32_t v)
    {
        return static_cast<size_t>(v);
    }

    template <typename... Args>
    [[nodiscard]] static std::optional<OpenGlError> ConsumeError(fmt::format_string<Args...> format, Args&&... args)
    {
        if (GlError e = OpenGl::GetError(); e != GlError::NoError)
        {
            std::string message = fmt::format("OpenGL error: {}. Context: ", e);
            fmt::format_to(std::back_inserter(message), format, std::forward<Args>(args)...);
            return OpenGlError(e, std::move(message), cpptrace::generate_raw_trace(1));
        }

        return std::nullopt;
    }

    template <typename T, typename... Args>
    [[nodiscard]] static tl::expected<std::decay_t<T>, OpenGlError>
    ValueOrError(T&& value, fmt::format_string<Args...> format, Args&&... args) noexcept
    {
        if (GlError e = OpenGl::GetError(); e != GlError::NoError)
        {
            return tl::unexpected{
                OpenGlError(e, fmt::format(format, std::forward<Args>(args)...), cpptrace::generate_raw_trace(1))};
        }

        return std::forward<T>(value);
    }

    template <typename Identifier>
    static void GenManyNE(std::span<Identifier> identifiers)
    {
        using Traits = detail::IdTraits<Identifier>;
        (*Traits::generator)(
            static_cast<GLsizei>(identifiers.size()),
            reinterpret_cast<Identifier::Repr*>(identifiers.data())  // NOLINT
        );
    }

    template <typename Identifier>
    [[nodiscard]] static std::optional<OpenGlError> GenManyCE(std::span<Identifier> identifiers)
    {
        using Traits = detail::IdTraits<Identifier>;
        (*Traits::generator)(
            static_cast<GLsizei>(identifiers.size()),
            reinterpret_cast<Identifier::Repr*>(identifiers.data())  // NOLINT
        );
        return ConsumeError(
            "{}(n: {}, array: {})",
            Traits::generator_name,
            identifiers.size(),
            static_cast<const void*>(identifiers.data()));
    }

    template <typename Identifier>
    static void GenMany(std::span<Identifier> identifiers)
    {
        ThrowIfError(GenManyCE(identifiers));
    }

    template <typename T>
    [[nodiscard]] static T GenOneNE()
    {
        T result{};
        GenManyNE(std::span{&result, 1});
        return result;
    }

    template <typename T>
    [[nodiscard]] static tl::expected<T, OpenGlError> GenOneCE()
    {
        T result{};
        if (auto maybe_error = GenManyCE(std::span{&result, 1}))
        {
            return tl::unexpected(std::move(maybe_error.value()));
        }

        return result;
    }

    template <typename T>
    [[nodiscard]] static T GenOne()
    {
        return TryTakeValue(GenOneCE<T>());
    }

    static constexpr auto GetInfoLog = [](const auto& fn, std::string_view fn_name, const auto& id, size_t length)
    {
        std::string log;
        log.resize(length);
        fn(id.GetValue(), static_cast<GLint>(length), nullptr, log.data());
        return ValueOrError(
            std::move(log),
            "{}(program: {}, bufSize: {}, length: nullptr, infoLog: {})",
            fn_name,
            id.GetValue(),
            log.size(),
            static_cast<const void*>(log.data()));
    };
};

GlError OpenGl::GetError() noexcept
{
    const auto error = glGetError();
    [[likely]] if (error == GL_NO_ERROR)
    {
        return GlError::NoError;
    }

    const size_t map_idx = kGlValueToGlError.FindKeyIndex(error);

    if (map_idx == ass::kInvalidIndex) return GlError::Unknown;

    return kGlValueToGlError.GetAtIndex(map_idx);
}

/************************************************** Buffers *******************************************************/

// Gen many

void OpenGl::GenBuffersNE(const std::span<GlBufferId>& buffers) noexcept
{
    Internal::GenManyNE(buffers);
}

std::optional<OpenGlError> OpenGl::GenBuffersCE(const std::span<GlBufferId>& buffers) noexcept
{
    return Internal::GenManyCE(buffers);
}

void OpenGl::GenBuffers(const std::span<GlBufferId>& buffers)
{
    Internal::GenMany(buffers);
}

// Gen one

GlBufferId OpenGl::GenBufferNE() noexcept
{
    return Internal::GenOneNE<GlBufferId>();
}

tl::expected<GlBufferId, OpenGlError> OpenGl::GenBufferCE() noexcept
{
    return Internal::GenOneCE<GlBufferId>();
}

GlBufferId OpenGl::GenBuffer()
{
    return Internal::GenOne<GlBufferId>();
}

// Bind

void OpenGl::BindBufferNE(GlBufferType target, GlBufferId buffer) noexcept
{
    glBindBuffer(ToGlValue(target), buffer.GetValue());
}

std::optional<OpenGlError> OpenGl::BindBufferCE(GlBufferType target, GlBufferId buffer) noexcept
{
    BindBufferNE(target, buffer);
    return Internal::ConsumeError("glBindBuffer(target: {}, buffer: {})", target, buffer.GetValue());
}

void OpenGl::BindBuffer(GlBufferType target, GlBufferId buffer)
{
    Internal::ThrowIfError(BindBufferCE(target, buffer));
}

// Buffer Data (with data)

void OpenGl::BufferDataNE(GlBufferType target, std::span<const uint8_t> data, GlUsage usage) noexcept
{
    glBufferData(ToGlValue(target), static_cast<GLsizei>(data.size()), data.data(), ToGlValue(usage));
}

std::optional<OpenGlError>
OpenGl::BufferDataCE(GlBufferType target, std::span<const uint8_t> data, GlUsage usage) noexcept
{
    BufferDataNE(target, data, usage);
    return Internal::ConsumeError(
        "glBufferData(target: {}, size: {}, data: {}, usage: {})",
        target,
        data.size(),
        static_cast<const void*>(data.data()),
        usage);
}

void OpenGl::BufferData(GlBufferType target, std::span<const uint8_t> data, GlUsage usage)
{
    Internal::ThrowIfError(BufferDataCE(target, data, usage));
}

// Buffer data (just size)

void OpenGl::BufferDataNE(GlBufferType target, size_t buffer_size, GlUsage usage) noexcept
{
    glBufferData(ToGlValue(target), static_cast<GLsizei>(buffer_size), nullptr, ToGlValue(usage));
}

std::optional<OpenGlError> OpenGl::BufferDataCE(GlBufferType target, size_t buffer_size, GlUsage usage) noexcept
{
    BufferDataNE(target, buffer_size, usage);
    return Internal::ConsumeError(
        "glBufferData(target: {}, size: {}, data: nullptr, usage: {})",
        target,
        buffer_size,
        usage);
}

void OpenGl::BufferData(GlBufferType target, size_t buffer_size, GlUsage usage)
{
    Internal::ThrowIfError(BufferDataCE(target, buffer_size, usage));
}

// Buffer sub data

void OpenGl::BufferSubDataNE(GlBufferType target, size_t offset_elements, std::span<const uint8_t> data) noexcept
{
    glBufferSubData(
        ToGlValue(target),
        static_cast<GLintptr>(offset_elements),
        static_cast<GLsizeiptr>(data.size()),
        data.data());
}

std::optional<OpenGlError>
OpenGl::BufferSubDataCE(GlBufferType target, size_t offset_elements, std::span<const uint8_t> data) noexcept
{
    BufferSubDataNE(target, offset_elements, data);
    return Internal::ConsumeError(
        "glBufferSubData(target: {}, offset: {}, size: {}, data: {})",
        target,
        offset_elements,
        data.size(),
        static_cast<const void*>(data.data()));
}

void OpenGl::BufferSubData(GlBufferType target, size_t offset_elements, std::span<const uint8_t> data)
{
    Internal::ThrowIfError(BufferSubDataCE(target, offset_elements, data));
}

// Delete

void OpenGl::DeleteBufferNE(GlBufferId buffer) noexcept
{
    glDeleteBuffers(1, &buffer.GetValue());
}

std::optional<OpenGlError> OpenGl::DeleteBufferCE(GlBufferId buffer) noexcept
{
    DeleteBufferNE(buffer);
    return Internal::ConsumeError("glDeleteBuffers(buffer: {})", buffer.GetValue());
}

void OpenGl::DeleteBuffer(GlBufferId buffer)
{
    Internal::ThrowIfError(DeleteBufferCE(buffer));
}

/*********************************************** Vertex Arrays ****************************************************/

// Gen many

void OpenGl::GenVertexArraysNE(const std::span<GlVertexArrayId>& arrays) noexcept
{
    Internal::GenManyNE(arrays);
}

std::optional<OpenGlError> OpenGl::GenVertexArraysCE(const std::span<GlVertexArrayId>& arrays) noexcept
{
    return Internal::GenManyCE(arrays);
}

void OpenGl::GenVertexArrays(const std::span<GlVertexArrayId>& arrays)
{
    Internal::GenMany(arrays);
}

// Gen one

GlVertexArrayId OpenGl::GenVertexArrayNE() noexcept
{
    return Internal::GenOneNE<GlVertexArrayId>();
}

tl::expected<GlVertexArrayId, OpenGlError> OpenGl::GenVertexArrayCE() noexcept
{
    return Internal::GenOneCE<GlVertexArrayId>();
}

GlVertexArrayId OpenGl::GenVertexArray()
{
    return Internal::GenOne<GlVertexArrayId>();
}

// Bind

void OpenGl::BindVertexArrayNE(GlVertexArrayId array) noexcept
{
    glBindVertexArray(array.GetValue());
}

std::optional<OpenGlError> OpenGl::BindVertexArrayCE(GlVertexArrayId array) noexcept
{
    BindVertexArrayNE(array);
    return Internal::ConsumeError("glBindVertexArray(array: {})", array.GetValue());
}

void OpenGl::BindVertexArray(GlVertexArrayId array)
{
    Internal::ThrowIfError(BindVertexArrayCE(array));
}

/********************************************** Vertex Arrays ****************************************************/

void OpenGl::DeleteVertexArrayNE(GlVertexArrayId array) noexcept
{
    glDeleteVertexArrays(1, &array.GetValue());
}

std::optional<OpenGlError> OpenGl::DeleteVertexArrayCE(GlVertexArrayId array) noexcept
{
    DeleteVertexArrayNE(array);
    return Internal::ConsumeError("glDeleteVertexArrays(n: 1, array: {})", array.GetValue());
}

void OpenGl::DeleteVertexArray(GlVertexArrayId array)
{
    Internal::ThrowIfError(DeleteVertexArrayCE(array));
}

/************************************************* Textures *******************************************************/

// Gen many

void OpenGl::GenTexturesNE(const std::span<GlTextureId>& textures) noexcept
{
    Internal::GenManyNE(textures);
}

std::optional<OpenGlError> OpenGl::GenTexturesCE(const std::span<GlTextureId>& textures) noexcept
{
    return Internal::GenManyCE(textures);
}

void OpenGl::GenTextures(const std::span<GlTextureId>& textures)
{
    Internal::GenMany(textures);
}

// Gen one

GlTextureId OpenGl::GenTextureNE() noexcept
{
    return Internal::GenOneNE<GlTextureId>();
}

tl::expected<GlTextureId, OpenGlError> OpenGl::GenTextureCE() noexcept
{
    return Internal::GenOneCE<GlTextureId>();
}

GlTextureId OpenGl::GenTexture()
{
    return Internal::GenOne<GlTextureId>();
}

// Bind

void OpenGl::BindTextureNE(GlTargetTextureType target, GlTextureId texture) noexcept
{
    glBindTexture(ToGlValue(target), texture.GetValue());
}

std::optional<OpenGlError> OpenGl::BindTextureCE(GlTargetTextureType target, GlTextureId texture) noexcept
{
    BindTextureNE(target, texture);
    return Internal::ConsumeError("glBindTexture(target: {}, texture: {})", target, texture.GetValue());
}

void OpenGl::BindTexture(GlTargetTextureType target, GlTextureId texture)
{
    Internal::ThrowIfError(BindTextureCE(target, texture));
}

// Set base level

void OpenGl::SetTextureBaseLevelNE(GlTargetTextureType target, size_t level) noexcept
{
    glTexParameteri(ToGlValue(target), GL_TEXTURE_BASE_LEVEL, static_cast<GLint>(level));
}

std::optional<OpenGlError> OpenGl::SetTextureBaseLevelCE(GlTargetTextureType target, size_t level) noexcept
{
    SetTextureBaseLevelNE(target, level);
    return Internal::ConsumeError(
        "glTexParameteri(target: {}, parameter: {}, param: {})",
        target,
        GlTextureParameterType::BaseLevel,
        level);
}

void OpenGl::SetTextureBaseLevel(GlTargetTextureType target, size_t level)
{
    Internal::ThrowIfError(SetTextureBaseLevelCE(target, level));
}

// Set depth texture compare mode

void OpenGl::SetDepthTextureCompareModeNE(GlTargetTextureType target, GlDepthTextureCompareMode mode) noexcept
{
    glTextureParameteri(ToGlValue(target), ToGlValue(GlTextureParameterType::CompareMode), ToGlValue(mode));
}

std::optional<OpenGlError> OpenGl::SetDepthTextureCompareModeCE(
    GlTargetTextureType target,
    GlDepthTextureCompareMode mode) noexcept
{
    SetDepthTextureCompareModeNE(target, mode);
    return Internal::ConsumeError(
        "glTexParameteri(target: {}, parameter: {}, value: {})",
        target,
        GlTextureParameterType::CompareMode,
        mode);
}

// Set depth texture compare function

void OpenGl::SetDepthTextureCompareFunctionNE(
    GlTargetTextureType target,
    GlDepthTextureCompareFunction function) noexcept
{
    glTexParameteri(ToGlValue(target), ToGlValue(GlTextureParameterType::ComapreFunction), ToGlValue(function));
}

[[nodiscard]] std::optional<OpenGlError> OpenGl::SetDepthTextureCompareFunctionCE(
    GlTargetTextureType target,
    GlDepthTextureCompareFunction function) noexcept
{
    SetDepthTextureCompareFunctionNE(target, function);
    return Internal::ConsumeError(
        "glTexParameteri(target: {}, parameter: {}, param: {})",
        target,
        GlTextureParameterType::ComapreFunction,
        function);
}

void OpenGl::SetDepthTextureCompareFunction(GlTargetTextureType target, GlDepthTextureCompareFunction function)
{
    Internal::ThrowIfError(SetDepthTextureCompareFunctionCE(target, function));
}

void OpenGl::SetDepthTextureCompareMode(GlTargetTextureType target, GlDepthTextureCompareMode mode)
{
    Internal::ThrowIfError(SetDepthTextureCompareModeCE(target, mode));
}

// Set border color: integers

void OpenGl::SetTextureBorderColorNE(
    GlTargetTextureType target,
    std::span<const GLint, 4> color,
    bool store_as_integer) noexcept
{
    if (store_as_integer)
    {
        glTexParameterIiv(ToGlValue(target), ToGlValue(GlTextureParameterType::BorderColor), color.data());
    }
    else
    {
        glTexParameteriv(ToGlValue(target), ToGlValue(GlTextureParameterType::BorderColor), color.data());
    }
}

std::optional<OpenGlError> OpenGl::SetTextureBorderColorCE(
    GlTargetTextureType target,
    std::span<const GLint, 4> color,
    bool store_as_integer) noexcept
{
    SetTextureBorderColorNE(target, color, store_as_integer);
    return Internal::ConsumeError(
        "(target: {}, parameter: {}, color: {})",
        store_as_integer ? "glTexParameterIiv" : "glTexParameteriv",
        GlTextureParameterType::BorderColor,
        target,
        color);
}

void OpenGl::SetTextureBorderColor(GlTargetTextureType target, std::span<const GLint, 4> color, bool store_as_integer)
{
    Internal::ThrowIfError(SetTextureBorderColorCE(target, color, store_as_integer));
}

// Set border color: unsigned integers

void OpenGl::SetTextureBorderColorNE(GlTargetTextureType target, std::span<const GLuint, 4> color) noexcept
{
    glTexParameterIuiv(ToGlValue(target), ToGlValue(GlTextureParameterType::BorderColor), color.data());
}

std::optional<OpenGlError> OpenGl::SetTextureBorderColorCE(
    GlTargetTextureType target,
    std::span<const GLuint, 4> color) noexcept
{
    SetTextureBorderColorNE(target, color);
    return Internal::ConsumeError(
        "glTexParameterIuiv(target: {}, parameter: {}, color: {})",
        target,
        GlTextureParameterType::BorderColor,
        color);
}

void OpenGl::SetTextureBorderColor(GlTargetTextureType target, std::span<const GLuint, 4> color)
{
    Internal::ThrowIfError(SetTextureBorderColorCE(target, color));
}

// Set border color: floats

void OpenGl::SetTextureBorderColorNE(GlTargetTextureType target, std::span<const GLfloat, 4> color) noexcept
{
    glTexParameterfv(ToGlValue(target), ToGlValue(GlTextureParameterType::BorderColor), color.data());
}

std::optional<OpenGlError> OpenGl::SetTextureBorderColorCE(
    GlTargetTextureType target,
    std::span<const GLfloat, 4> color) noexcept
{
    SetTextureBorderColorNE(target, color);
    return Internal::ConsumeError(
        "glTexParameterfv(target: {}, parameter: {}, color: {})",
        target,
        GlTextureParameterType::BorderColor,
        color);
}

void OpenGl::SetTextureBorderColor(GlTargetTextureType target, std::span<const GLfloat, 4> color)
{
    Internal::ThrowIfError(SetTextureBorderColorCE(target, color));
}

// Set lod bias

void OpenGl::SetTextureLODBiasNE(GlTargetTextureType target, float bias) noexcept
{
    glTexParameterf(ToGlValue(target), ToGlValue(GlTextureParameterType::LoadBias), bias);
}

std::optional<OpenGlError> OpenGl::SetTextureLODBiasCE(GlTargetTextureType target, float bias) noexcept
{
    SetTextureLODBiasNE(target, bias);
    return Internal::ConsumeError(
        "glTexParameterf(target: {}, parameter: {}, bias: {})",
        target,
        GlTextureParameterType::LoadBias,
        bias);
}

void OpenGl::SetTextureLODBias(GlTargetTextureType target, float bias)
{
    Internal::ThrowIfError(SetTextureLODBiasCE(target, bias));
}

// Set texture wrap mode

void OpenGl::SetTextureWrapNE(GlTargetTextureType target, GlTextureWrapAxis wrap, GlTextureWrapMode mode) noexcept
{
    glTexParameteri(ToGlValue(target), ToGlValue(wrap), ToGlValue(mode));
}

[[nodiscard]] std::optional<OpenGlError>
OpenGl::SetTextureWrapCE(GlTargetTextureType target, GlTextureWrapAxis wrap, GlTextureWrapMode mode) noexcept
{
    SetTextureWrapNE(target, wrap, mode);
    return Internal::ConsumeError("glTexParameteri(target: {}, axis: {}, wrap: {})", target, wrap, mode);
}

void OpenGl::SetTextureWrap(GlTargetTextureType target, GlTextureWrapAxis wrap, GlTextureWrapMode mode)
{
    Internal::ThrowIfError(SetTextureWrapCE(target, wrap, mode));
}

// Set texture minification filter

void OpenGl::SetTextureMinFilterNE(GlTargetTextureType target, GlTextureFilter filter) noexcept
{
    glTexParameteri(ToGlValue(target), ToGlValue(GlTextureParameterType::MinificationFilter), ToGlValue(filter));
}

std::optional<OpenGlError> OpenGl::SetTextureMinFilterCE(GlTargetTextureType target, GlTextureFilter filter) noexcept
{
    SetTextureMinFilterNE(target, filter);
    return Internal::ConsumeError(
        "glTexParameteri(target: {}, parameter: {}, filter: {})",
        target,
        GlTextureParameterType::MinificationFilter,
        filter);
}

void OpenGl::SetTextureMinFilter(GlTargetTextureType target, GlTextureFilter filter)
{
    Internal::ThrowIfError(SetTextureMinFilterCE(target, filter));
}

// Set texture magnification filter

void OpenGl::SetTextureMagFilterNE(GlTargetTextureType target, GlTextureFilter filter) noexcept
{
    glTexParameteri(ToGlValue(target), ToGlValue(GlTextureParameterType::MagnificationFilter), ToGlValue(filter));
}

std::optional<OpenGlError> OpenGl::SetTextureMagFilterCE(GlTargetTextureType target, GlTextureFilter filter) noexcept
{
    SetTextureMagFilterNE(target, filter);
    return Internal::ConsumeError(
        "glTexParameteri(target: {}, parameter: {}, filter: {})",
        target,
        GlTextureParameterType::MagnificationFilter,
        filter);
}

void OpenGl::SetTextureMagFilter(GlTargetTextureType target, GlTextureFilter filter)
{
    Internal::ThrowIfError(SetTextureMagFilterCE(target, filter));
}

// Tex image 2d

void OpenGl::TexImage2dNE(
    GlTargetTextureType target,
    size_t level_of_detail,
    GLint internal_format,
    size_t width,
    size_t height,
    GLint data_format,
    GLenum pixel_data_type,
    const void* pixels) noexcept
{
    glTexImage2D(
        ToGlValue(target),
        static_cast<GLint>(level_of_detail),
        internal_format,
        static_cast<GLsizei>(width),
        static_cast<GLsizei>(height),
        0,
        data_format,
        pixel_data_type,
        pixels);
}

std::optional<OpenGlError> OpenGl::TexImage2dCE(
    GlTargetTextureType target,
    size_t level_of_detail,
    GLint internal_format,
    size_t width,
    size_t height,
    GLint data_format,
    GLenum pixel_data_type,
    const void* pixels) noexcept
{
    TexImage2dNE(target, level_of_detail, internal_format, width, height, data_format, pixel_data_type, pixels);
    return Internal::ConsumeError(
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

void OpenGl::TexImage2d(
    GlTargetTextureType target,
    size_t level_of_detail,
    GLint internal_format,
    size_t width,
    size_t height,
    GLint data_format,
    GLenum pixel_data_type,
    const void* pixels)
{
    return Internal::ThrowIfError(
        TexImage2dCE(target, level_of_detail, internal_format, width, height, data_format, pixel_data_type, pixels));
}

// Delete

void OpenGl::DeleteTextureNE(GlTextureId texture) noexcept
{
    glDeleteTextures(1, &texture.GetValue());
}

std::optional<OpenGlError> OpenGl::DeleteTextureCE(GlTextureId texture) noexcept
{
    DeleteTextureNE(texture);
    return Internal::ConsumeError("glDeleteTextures(texture: {})", texture.GetValue());
}

void OpenGl::DeleteTexture(GlTextureId texture)
{
    Internal::ThrowIfError(DeleteTextureCE(texture));
}

/************************************************** Framebuffers **************************************************/

// Gen many

void OpenGl::GenFramebuffersNE(const std::span<GlFramebufferId>& framebuffers) noexcept
{
    Internal::GenManyNE(framebuffers);
}

std::optional<OpenGlError> OpenGl::GenFramebuffersCE(const std::span<GlFramebufferId>& framebuffers) noexcept
{
    return Internal::GenManyCE(framebuffers);
}

void OpenGl::GenFramebuffers(const std::span<GlFramebufferId>& framebuffers)
{
    Internal::GenMany(framebuffers);
}

// Gen one

GlFramebufferId OpenGl::GenFramebufferNE() noexcept
{
    return Internal::GenOneNE<GlFramebufferId>();
}

tl::expected<GlFramebufferId, OpenGlError> OpenGl::GenFramebufferCE() noexcept
{
    return Internal::GenOneCE<GlFramebufferId>();
}

GlFramebufferId OpenGl::GenFramebuffer()
{
    return Internal::GenOne<GlFramebufferId>();
}

// Bind

void OpenGl::BindFramebufferNE(GlFramebufferBindTarget target, GlFramebufferId framebuffer) noexcept
{
    glBindFramebuffer(ToGlValue(target), framebuffer.GetValue());
}

std::optional<OpenGlError> OpenGl::BindFramebufferCE(
    GlFramebufferBindTarget target,
    GlFramebufferId framebuffer) noexcept
{
    BindFramebufferNE(target, framebuffer);
    return Internal::ConsumeError("glBindFramebuffer(target: {}, framebuffer: {})", target, framebuffer.GetValue());
}

void OpenGl::BindFramebuffer(GlFramebufferBindTarget target, GlFramebufferId framebuffer)
{
    Internal::ThrowIfError(BindFramebufferCE(target, framebuffer));
}

// Attach texture

void OpenGl::FramebufferTexture2DNE(
    GlFramebufferBindTarget target,
    GlFramebufferAttachment attachment,
    GlTargetTextureType textarget,
    GlTextureId texture,
    size_t level) noexcept
{
    glFramebufferTexture2D(
        ToGlValue(target),
        ToGlValue(attachment),
        ToGlValue(textarget),
        texture.GetValue(),
        static_cast<GLint>(level));
}

std::optional<OpenGlError> OpenGl::FramebufferTexture2DCE(
    GlFramebufferBindTarget target,
    GlFramebufferAttachment attachment,
    GlTargetTextureType textarget,
    GlTextureId texture,
    size_t level) noexcept
{
    FramebufferTexture2DNE(target, attachment, textarget, texture, level);
    return Internal::ConsumeError(
        "glFramebufferTexture2D(target: {}, attachment: {}, textarget: {}, texture: {}, level: {})",
        target,
        attachment,
        textarget,
        texture.GetValue(),
        level);
}

void OpenGl::FramebufferTexture2D(
    GlFramebufferBindTarget target,
    GlFramebufferAttachment attachment,
    GlTargetTextureType textarget,
    GlTextureId texture,
    size_t level)
{
    Internal::ThrowIfError(FramebufferTexture2DCE(target, attachment, textarget, texture, level));
}

// Attach renderbuffer

void OpenGl::FramebufferRenderbufferNE(
    GlFramebufferBindTarget target,
    GlFramebufferAttachment attachment,
    GlRenderbufferId renderbuffer) noexcept
{
    glFramebufferRenderbuffer(ToGlValue(target), ToGlValue(attachment), GL_RENDERBUFFER, renderbuffer.GetValue());
}

std::optional<OpenGlError> OpenGl::FramebufferRenderbufferCE(
    GlFramebufferBindTarget target,
    GlFramebufferAttachment attachment,
    GlRenderbufferId renderbuffer) noexcept
{
    FramebufferRenderbufferNE(target, attachment, renderbuffer);
    return Internal::ConsumeError(
        "glFramebufferRenderbuffer(target: {}, attachment: {}, renderbuffer: {})",
        target,
        attachment,
        renderbuffer.GetValue());
}

void OpenGl::FramebufferRenderbuffer(
    GlFramebufferBindTarget target,
    GlFramebufferAttachment attachment,
    GlRenderbufferId renderbuffer)
{
    Internal::ThrowIfError(FramebufferRenderbufferCE(target, attachment, renderbuffer));
}

// Delete

void OpenGl::DeleteFramebufferNE(GlFramebufferId framebuffer) noexcept
{
    glDeleteFramebuffers(1, &framebuffer.GetValue());
}

std::optional<OpenGlError> OpenGl::DeleteFramebufferCE(GlFramebufferId framebuffer) noexcept
{
    DeleteFramebufferNE(framebuffer);
    return Internal::ConsumeError("glDeleteFramebuffers(framebuffer: {})", framebuffer.GetValue());
}

void OpenGl::DeleteFramebuffer(GlFramebufferId framebuffer)
{
    Internal::ThrowIfError(DeleteFramebufferCE(framebuffer));
}

/************************************************* RenderBuffers **************************************************/

// Gen many

void OpenGl::GenRenderbuffersNE(const std::span<GlRenderbufferId>& renderbuffers) noexcept
{
    Internal::GenManyNE(renderbuffers);
}

std::optional<OpenGlError> OpenGl::GenRenderbuffersCE(const std::span<GlRenderbufferId>& renderbuffers) noexcept
{
    return Internal::GenManyCE(renderbuffers);
}

void OpenGl::GenRenderbuffers(const std::span<GlRenderbufferId>& renderbuffers)
{
    Internal::GenMany(renderbuffers);
}

// Gen one

GlRenderbufferId OpenGl::GenRenderbufferNE() noexcept
{
    return Internal::GenOneNE<GlRenderbufferId>();
}

tl::expected<GlRenderbufferId, OpenGlError> OpenGl::GenRenderbufferCE() noexcept
{
    return Internal::GenOneCE<GlRenderbufferId>();
}

GlRenderbufferId OpenGl::GenRenderbuffer()
{
    return Internal::GenOne<GlRenderbufferId>();
}

// Bind

void OpenGl::BindRenderbufferNE(GlRenderbufferId renderbuffer) noexcept
{
    glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer.GetValue());
}

std::optional<OpenGlError> OpenGl::BindRenderbufferCE(GlRenderbufferId renderbuffer) noexcept
{
    BindRenderbufferNE(renderbuffer);
    return Internal::ConsumeError("glBindRenderbuffer(renderbuffer: {})", renderbuffer.GetValue());
}

void OpenGl::BindRenderbuffer(GlRenderbufferId renderbuffer)
{
    Internal::ThrowIfError(BindRenderbufferCE(renderbuffer));
}

// Storage

void OpenGl::RenderbufferStorageNE(GlTextureInternalFormat format, const edt::Vec2<size_t>& size) noexcept
{
    const auto size_i = size.Cast<GLsizei>();
    glRenderbufferStorage(GL_RENDERBUFFER, ToGlValue(format), size_i.x(), size_i.y());
}

std::optional<OpenGlError> OpenGl::RenderbufferStorageCE(
    GlTextureInternalFormat format,
    const edt::Vec2<size_t>& size) noexcept
{
    RenderbufferStorageNE(format, size);
    return Internal::ConsumeError(
        "glRenderbufferStorage(internal_format: {}, width: {}, height: {})",
        format,
        size.x(),
        size.y());
}

void OpenGl::RenderbufferStorage(GlTextureInternalFormat format, const edt::Vec2<size_t>& size)
{
    Internal::ThrowIfError(RenderbufferStorageCE(format, size));
}

// Delete

void OpenGl::DeleteRenderbufferNE(GlRenderbufferId renderbuffer) noexcept
{
    glDeleteRenderbuffers(1, &renderbuffer.GetValue());
}

std::optional<OpenGlError> OpenGl::DeleteRenderbufferCE(GlRenderbufferId renderbuffer) noexcept
{
    DeleteRenderbufferNE(renderbuffer);
    return Internal::ConsumeError("glDeleteRenderbuffers(renderbuffer: {})", renderbuffer.GetValue());
}

void OpenGl::DeleteRenderbuffer(GlRenderbufferId renderbuffer)
{
    Internal::ThrowIfError(DeleteRenderbufferCE(renderbuffer));
}

/************************************************** Shaders *******************************************************/

// Create

GlShaderId OpenGl::CreateShaderNE(GlShaderType type) noexcept
{
    return GlShaderId::FromValue(glCreateShader(ToGlValue(type)));
}

tl::expected<GlShaderId, OpenGlError> OpenGl::CreateShaderCE(GlShaderType type) noexcept
{
    return Internal::ValueOrError(CreateShaderNE(type), "glCreateShader(type: {})", type);
}

GlShaderId OpenGl::CreateShader(GlShaderType type)
{
    return Internal::TryTakeValue(CreateShaderCE(type));
}

// Shader source

void OpenGl::ShaderSourceNE(GlShaderId shader, std::span<const std::string_view> sources) noexcept
{
    Internal::ShaderSourceCollector<30> c;
    auto [sources_span, lengths_span] = c.Fill(sources);
    glShaderSource(shader.GetValue(), static_cast<GLsizei>(sources.size()), sources_span.data(), lengths_span.data());
}

std::optional<OpenGlError> OpenGl::ShaderSourceCE(GlShaderId shader, std::span<const std::string_view> sources) noexcept
{
    Internal::ShaderSourceCollector<30> c;
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

// Compile shader

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

// Get compile status

tl::expected<bool, OpenGlError> OpenGl::GetShaderCompileStatusCE(GlShaderId shader) noexcept
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

// Get shader log length

tl::expected<size_t, OpenGlError> OpenGl::GetShaderLogLengthCE(GlShaderId shader) noexcept
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

// Get shader log

tl::expected<std::string, OpenGlError> OpenGl::GetShaderLogCE(GlShaderId shader) noexcept
{
    return GetShaderLogLengthCE(shader).and_then(
        std::bind_front(Internal::GetInfoLog, glGetShaderInfoLog, "glGetShaderInfoLog", shader));
}

std::string OpenGl::GetShaderLog(GlShaderId shader)
{
    return Internal::TryTakeValue(GetShaderLogCE(shader));
}

// Set uniform: float

void OpenGl::SetUniformNE(uint32_t location, int32_t v) noexcept
{
    glUniform1i(static_cast<GLint>(location), v);
}

std::optional<OpenGlError> OpenGl::SetUniformCE(uint32_t location, int32_t v) noexcept
{
    SetUniformNE(location, v);
    return Internal::ConsumeError("glUniform1i(location: {}, value: {})", location, v);
}

void OpenGl::SetUniform(uint32_t location, int32_t v)
{
    Internal::ThrowIfError(SetUniformCE(location, v));
}

void OpenGl::SetUniformNE(uint32_t location, uint32_t v) noexcept
{
    glUniform1ui(static_cast<GLint>(location), v);
}

std::optional<OpenGlError> OpenGl::SetUniformCE(uint32_t location, uint32_t v) noexcept
{
    SetUniformNE(location, v);
    return Internal::ConsumeError("glUniform1ui(location: {}, value: {})", location, v);
}

void OpenGl::SetUniform(uint32_t location, uint32_t v)
{
    Internal::ThrowIfError(SetUniformCE(location, v));
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

// Set uniform: vec2

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

// Set uniform: vec3

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

// Set uniform: vec4

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

// Set uniform: mat3

void OpenGl::SetUniformNE(uint32_t location, const Mat3f& m, bool transpose) noexcept
{
    glUniformMatrix3fv(static_cast<GLint>(location), 1, Internal::CastBool(transpose), m.data());
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

// Set uniform: mat4

void OpenGl::SetUniformNE(uint32_t location, const Mat4f& m, bool transpose) noexcept
{
    glUniformMatrix4fv(static_cast<GLint>(location), 1, Internal::CastBool(transpose), m.data());
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

// Delete

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

/************************************************** Program *******************************************************/

// Create

GlProgramId OpenGl::CreateProgramNE() noexcept
{
    return GlProgramId::FromValue(glCreateProgram());
}

tl::expected<GlProgramId, OpenGlError> OpenGl::CreateProgramCE() noexcept
{
    return Internal::ValueOrError(CreateProgramNE(), "glCreateProgram()");
}

GlProgramId OpenGl::CreateProgram()
{
    return Internal::TryTakeValue(CreateProgramCE());
}

// Attach shader

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

// Link

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

// Link status

bool OpenGl::GetProgramLinkStatusNE(GlProgramId program) noexcept
{
    return GetProgramIntParameterNE(program, GlProgramIntParameter::LinkStatus) == GL_TRUE;
}

tl::expected<bool, OpenGlError> OpenGl::GetProgramLinkStatusCE(GlProgramId program) noexcept
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

// Program int parameter

int32_t OpenGl::GetProgramIntParameterNE(GlProgramId program, GlProgramIntParameter parameter) noexcept
{
    int32_t value{};
    glGetProgramiv(program.GetValue(), ToGlValue(parameter), &value);
    return value;
}

tl::expected<int32_t, OpenGlError> OpenGl::GetProgramIntParameterCE(
    GlProgramId program,
    GlProgramIntParameter parameter) noexcept
{
    return Internal::ValueOrError(
        GetProgramIntParameterNE(program, parameter),
        "glGetProgramiv(program: {}, pname: {})",
        program.GetValue(),
        parameter);
}

int32_t OpenGl::GetProgramIntParameter(GlProgramId program, GlProgramIntParameter parameter)
{
    return Internal::TryTakeValue(GetProgramIntParameterCE(program, parameter));
}

// Get work group size of compute shader

Vec3i OpenGl::GetProgramWorkGroupSizeNE(GlProgramId program) noexcept
{
    Vec3i result{};
    glGetProgramiv(program.GetValue(), ToGlValue(GlProgramIntParameter::ComputeWorkGroupSize), result.data());
    return result;
}

tl::expected<Vec3i, OpenGlError> OpenGl::GetProgramWorkGroupSizeCE(GlProgramId program) noexcept
{
    return Internal::ValueOrError(
        GetProgramWorkGroupSizeNE(program),
        "glGetProgramiv(program: {}, pname: {})",
        program.GetValue(),
        ToGlValue(GlProgramIntParameter::ComputeWorkGroupSize));
}

Vec3i OpenGl::GetProgramWorkGroupSize(GlProgramId program)
{
    return Internal::TryTakeValue(GetProgramWorkGroupSizeCE(program));
}

// Active attributes count

size_t OpenGl::GetProgramActiveAttributesCountNE(GlProgramId program) noexcept
{
    return static_cast<size_t>(GetProgramIntParameterNE(program, GlProgramIntParameter::ActiveAttributes));
}

tl::expected<size_t, OpenGlError> OpenGl::GetProgramActiveAttributesCountCE(GlProgramId program) noexcept
{
    return GetProgramIntParameterCE(program, GlProgramIntParameter::ActiveAttributes).and_then(Internal::CastToSizeT);
}

size_t OpenGl::GetProgramActiveAttributesCount(GlProgramId program)
{
    return Internal::TryTakeValue(GetProgramActiveAttributesCountCE(program));
}

// Max attribute name length

size_t OpenGl::GetProgramActiveAttributeMaxNameLengthNE(GlProgramId program) noexcept
{
    return static_cast<size_t>(GetProgramIntParameterNE(program, GlProgramIntParameter::ActiveAttributeMaxLength));
}

tl::expected<size_t, OpenGlError> OpenGl::GetProgramActiveAttributeMaxNameLengthCE(GlProgramId program) noexcept
{
    return GetProgramIntParameterCE(program, GlProgramIntParameter::ActiveAttributeMaxLength)
        .and_then(Internal::CastToSizeT);
}

size_t OpenGl::GetProgramActiveAttributeMaxNameLength(GlProgramId program)
{
    return Internal::TryTakeValue(GetProgramActiveAttributeMaxNameLengthCE(program));
}

// Get attribute info

void OpenGl::GetActiveAttributeNE(
    GlProgramId program,
    size_t attribute_index,
    size_t name_buffer_size,
    size_t& out_written_to_name_buffer,
    size_t& out_attribute_size,
    GlVertexAttributeType& out_attribute_type,
    char* out_name_buffer) noexcept
{
    GLint size = 0;
    GLenum type = ToGlValue(GlVertexAttributeType::Float);
    GLint written = 0;
    glGetActiveAttrib(
        program.GetValue(),
        static_cast<GLuint>(attribute_index),
        static_cast<GLsizei>(name_buffer_size),
        &written,
        &size,
        &type,
        out_name_buffer);

    out_written_to_name_buffer = static_cast<size_t>(std::max<GLint>(0, written));
    out_attribute_size = static_cast<size_t>(std::max<GLint>(0, written));
    out_attribute_type = FromGlValue(type);
}

std::optional<OpenGlError> OpenGl::GetActiveAttributeCE(
    GlProgramId program,
    size_t attribute_index,
    size_t name_buffer_size,
    size_t& out_written_to_name_buffer,
    size_t& out_attribute_size,
    GlVertexAttributeType& out_attribute_type,
    char* out_name_buffer) noexcept
{
    GetActiveAttributeNE(
        program,
        attribute_index,
        name_buffer_size,
        out_written_to_name_buffer,
        out_attribute_size,
        out_attribute_type,
        out_name_buffer);
    return Internal::ConsumeError(
        "glGetActiveAttrib(program: {}, index: {}, bufSize: {})",
        program.GetValue(),
        attribute_index,
        name_buffer_size);
}

void OpenGl::GetActiveAttribute(
    GlProgramId program,
    size_t attribute_index,
    size_t name_buffer_size,
    size_t& out_written_to_name_buffer,
    size_t& out_attribute_size,
    GlVertexAttributeType& out_attribute_type,
    char* out_name_buffer) noexcept
{
    Internal::ThrowIfError(GetActiveAttributeCE(
        program,
        attribute_index,
        name_buffer_size,
        out_written_to_name_buffer,
        out_attribute_size,
        out_attribute_type,
        out_name_buffer));
}

// Get attribute location by name
int32_t OpenGl::GetAttributeLocationNE(GlProgramId program, std::string_view attribute_name) noexcept
{
    return glGetAttribLocation(program.GetValue(), attribute_name.data());
}

tl::expected<size_t, OpenGlError> OpenGl::GetAttributeLocationCE(
    GlProgramId program,
    std::string_view attribute_name) noexcept
{
    int32_t location = GetAttributeLocationNE(program, attribute_name);
    return Internal::ValueOrError(
        static_cast<size_t>(location),
        "glGetAttribLocation(program: {}, name: {})",
        program.GetValue(),
        attribute_name);
}

size_t OpenGl::GetAttributeLocation(GlProgramId program, std::string_view attribute_name)
{
    return Internal::TryTakeValue(GetAttributeLocationCE(program, attribute_name));
}

// Get uniforms count

size_t OpenGl::GetProgramActiveUniformsCountNE(GlProgramId program) noexcept
{
    auto count = GetProgramIntParameterNE(program, GlProgramIntParameter::ActiveUniforms);
    return static_cast<size_t>(std::max<int32_t>(0, count));
}

tl::expected<size_t, OpenGlError> OpenGl::GetProgramActiveUniformsCountCE(GlProgramId program) noexcept
{
    return GetProgramIntParameterCE(program, GlProgramIntParameter::ActiveUniforms).and_then(Internal::CastToSizeT);
}

size_t OpenGl::GetProgramActiveUniformsCount(GlProgramId program)
{
    return Internal::TryTakeValue(GetProgramActiveUniformsCountCE(program));
}

// Uniform max name length

size_t OpenGl::GetProgramActiveUniformMaxNameLengthNE(GlProgramId program) noexcept
{
    return static_cast<size_t>(GetProgramIntParameterNE(program, GlProgramIntParameter::ActiveUniformMaxLength));
}

tl::expected<size_t, OpenGlError> OpenGl::GetProgramActiveUniformMaxNameLengthCE(GlProgramId program) noexcept
{
    return GetProgramIntParameterCE(program, GlProgramIntParameter::ActiveUniformMaxLength)
        .and_then(Internal::CastToSizeT);
}

size_t OpenGl::GetProgramActiveUniformMaxNameLength(GlProgramId program)
{
    return Internal::TryTakeValue(GetProgramActiveUniformMaxNameLengthCE(program));
}

// Get active uniform

void OpenGl::GetActiveUniformNE(
    GlProgramId program,
    size_t uniform_index,
    size_t name_buffer_size,
    size_t& out_written_to_name_buffer,
    size_t& out_uniform_size,
    GlUniformType& out_uniform_type,
    char* out_name_buffer) noexcept
{
    GLint size = 0;
    GLenum type = ToGlValue(GlUniformType::Float);
    GLint written = 0;
    glGetActiveUniform(
        program.GetValue(),
        static_cast<GLuint>(uniform_index),
        static_cast<GLsizei>(name_buffer_size),
        &written,
        &size,
        &type,
        out_name_buffer);

    out_written_to_name_buffer = static_cast<size_t>(std::max<GLint>(0, written));
    out_uniform_size = static_cast<size_t>(std::max<GLint>(0, size));
    out_uniform_type = detail::kGlUniformTypeToGlValue.from_gl_enum.Get(type);
}

std::optional<OpenGlError> OpenGl::GetActiveUniformCE(
    GlProgramId program,
    size_t uniform_index,
    size_t name_buffer_size,
    size_t& out_written_to_name_buffer,
    size_t& out_uniform_size,
    GlUniformType& out_uniform_type,
    char* out_name_buffer) noexcept
{
    GetActiveUniformNE(
        program,
        uniform_index,
        name_buffer_size,
        out_written_to_name_buffer,
        out_uniform_size,
        out_uniform_type,
        out_name_buffer);
    return Internal::ConsumeError(
        "glGetActiveUniform(program: {}, index: {}, bufSize: {})",
        program.GetValue(),
        uniform_index,
        name_buffer_size);
}

void OpenGl::GetActiveUniform(
    GlProgramId program,
    size_t uniform_index,
    size_t name_buffer_size,
    size_t& out_written_to_name_buffer,
    size_t& out_uniform_size,
    GlUniformType& out_uniform_type,
    char* out_name_buffer) noexcept
{
    Internal::ThrowIfError(GetActiveUniformCE(
        program,
        uniform_index,
        name_buffer_size,
        out_written_to_name_buffer,
        out_uniform_size,
        out_uniform_type,
        out_name_buffer));
}

// Log length

size_t OpenGl::GetProgramLogLengthNE(GlProgramId program) noexcept
{
    GLint log_length{};
    glGetProgramiv(program.GetValue(), GL_INFO_LOG_LENGTH, &log_length);
    return static_cast<size_t>(log_length);
}

tl::expected<size_t, OpenGlError> OpenGl::GetProgramLogLengthCE(GlProgramId program) noexcept
{
    GLint log_length{};
    glGetProgramiv(program.GetValue(), GL_INFO_LOG_LENGTH, &log_length);
    return Internal::ValueOrError(log_length, "glGetProgramiv(program: {}, GL_INFO_LOG_LENGTH)", program.GetValue());
}

size_t OpenGl::GetProgramLogLength(GlProgramId program)
{
    return Internal::TryTakeValue(GetProgramLogLengthCE(program));
}

// Log

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

tl::expected<std::string, OpenGlError> OpenGl::GetProgramLogCE(GlProgramId program) noexcept
{
    return GetProgramLogLengthCE(program).and_then(
        std::bind_front(Internal::GetInfoLog, glGetProgramInfoLog, "glGetProgramInfoLog", program));
}

std::string OpenGl::GetProgramLog(GlProgramId program)
{
    return Internal::TryTakeValue(GetProgramLogCE(program));
}

// Use

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

// Get uniform location

GLint OpenGl::GetUniformLocationNE(GlProgramId program, const char* name) noexcept
{
    return glGetUniformLocation(program.GetValue(), name);
}

tl::expected<GLint, OpenGlError> OpenGl::GetUniformLocationCE(GlProgramId program, const char* name) noexcept
{
    return Internal::ValueOrError(
        GetUniformLocationNE(program, name),
        "glGetUniformLocation(program: {}, name: {})",
        program.GetValue(),
        name);
}

GLint OpenGl::GetUniformLocation(GlProgramId program, const char* name)
{
    return Internal::TryTakeValue(GetUniformLocationCE(program, name));
}

// Delete

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

/*************************************************** Clear ********************************************************/

// Set clear color

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

// Set clear color: vector

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

// Clear

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

/************************************************ Face Culling ****************************************************/

// Enable

void OpenGl::EnableFaceCullingNE(bool value) noexcept
{
    if (value)
    {
        glEnable(GL_CULL_FACE);
    }
    else
    {
        glDisable(GL_CULL_FACE);
    }
}

std::optional<OpenGlError> OpenGl::EnableFaceCullingCE(bool value) noexcept
{
    EnableFaceCullingNE(value);
    return Internal::ConsumeError("{}", value ? "glEnable(GL_CULL_FACE)" : "glDisable(GL_CULL_FACE)");
}

void OpenGl::EnableFaceCulling(bool value)
{
    Internal::ThrowIfError(EnableFaceCullingCE(value));
}

// Set mode

void OpenGl::CullFaceNE(GlCullFaceMode mode) noexcept
{
    glCullFace(ToGlValue(mode));
}

std::optional<OpenGlError> OpenGl::CullFaceCE(GlCullFaceMode mode) noexcept
{
    CullFaceNE(mode);
    return Internal::ConsumeError("glCullFace", mode);
}

void OpenGl::CullFace(GlCullFaceMode mode)
{
    Internal::ThrowIfError(CullFaceCE(mode));
}

/****************************************** Vertex Attribute Pointer **********************************************/

void OpenGl::VertexAttribPointerNE(
    size_t index,
    size_t size,
    GlVertexAttribComponentType type,
    bool normalized,
    size_t stride,
    const void* pointer) noexcept
{
    glVertexAttribPointer(
        static_cast<GLuint>(index),
        static_cast<GLint>(size),
        ToGlValue(type),
        Internal::CastBool(normalized),
        static_cast<GLsizei>(stride),
        pointer);
}

std::optional<OpenGlError> OpenGl::VertexAttribPointerCE(
    size_t index,
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
    size_t index,
    size_t size,
    GlVertexAttribComponentType type,
    bool normalized,
    size_t stride,
    const void* pointer)
{
    Internal::ThrowIfError(VertexAttribPointerCE(index, size, type, normalized, stride, pointer));
}

// Integer version

void OpenGl::VertexAttribIPointerNE(
    size_t index,
    size_t size,
    GlVertexAttribComponentType type,
    size_t stride,
    const void* pointer) noexcept
{
    glVertexAttribIPointer(
        static_cast<GLuint>(index),
        static_cast<GLint>(size),
        ToGlValue(type),
        static_cast<GLsizei>(stride),
        pointer);
}

std::optional<OpenGlError> OpenGl::VertexAttribIPointerCE(
    size_t index,
    size_t size,
    GlVertexAttribComponentType type,
    size_t stride,
    const void* pointer) noexcept
{
    VertexAttribIPointerNE(index, size, type, stride, pointer);
    return Internal::ConsumeError(
        "glVertexAttribIPointer(index: {}, size: {}, type: {}, stride: {}, pointer: {})",
        index,
        size,
        type,
        stride,
        pointer);
}

void OpenGl::VertexAttribIPointer(
    size_t index,
    size_t size,
    GlVertexAttribComponentType type,
    size_t stride,
    const void* pointer)
{
    Internal::ThrowIfError(VertexAttribIPointerCE(index, size, type, stride, pointer));
}

/****************************************************** Draw ******************************************************/

// Draw elements

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

// Draw elements instanced

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

// Draw arrays

void OpenGl::DrawArraysNE(GlPrimitiveType mode, size_t first_index, size_t indices_count) noexcept
{
    glDrawArrays(ToGlValue(mode), static_cast<GLint>(first_index), static_cast<GLsizei>(indices_count));
}

std::optional<OpenGlError> OpenGl::DrawArraysCE(GlPrimitiveType mode, size_t first_index, size_t indices_count) noexcept
{
    DrawArraysNE(mode, first_index, indices_count);
    return Internal::ConsumeError("glDrawArrays(mode: {}, first: {}, count: {})", mode, first_index, indices_count);
}

void OpenGl::DrawArrays(GlPrimitiveType mode, size_t first_index, size_t indices_count)
{
    Internal::ThrowIfError(DrawArraysCE(mode, first_index, indices_count));
}

// Draw arrays instanced

void OpenGl::DrawArraysInstancedNE(
    GlPrimitiveType mode,
    size_t first_index,
    size_t indices_count,
    size_t instances_count) noexcept
{
    glDrawArraysInstanced(
        ToGlValue(mode),
        static_cast<GLint>(first_index),
        static_cast<GLsizei>(indices_count),
        static_cast<GLsizei>(instances_count));
}

std::optional<OpenGlError> OpenGl::DrawArraysInstancedCE(
    GlPrimitiveType mode,
    size_t first_index,
    size_t indices_count,
    size_t instances_count) noexcept
{
    DrawArraysInstancedNE(mode, first_index, indices_count, instances_count);
    return Internal::ConsumeError(
        "glDrawArraysInstanced(mode: {}, first: {}, count: {}, instancecount: {})",
        mode,
        first_index,
        indices_count,
        instances_count);
}

void OpenGl::DrawArraysInstanced(GlPrimitiveType mode, size_t first_index, size_t indices_count, size_t instances_count)
{
    Internal::ThrowIfError(DrawArraysInstancedCE(mode, first_index, indices_count, instances_count));
}

/******************************************************************************************************************/

void OpenGl::EnableVertexAttribArrayNE(size_t index) noexcept
{
    glEnableVertexAttribArray(static_cast<GLuint>(index));
}

std::optional<OpenGlError> OpenGl::EnableVertexAttribArrayCE(size_t index) noexcept
{
    EnableVertexAttribArrayNE(index);
    return Internal::ConsumeError("glEnableVertexAttribArray(index: {})", index);
}

void OpenGl::EnableVertexAttribArray(size_t index)
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

void OpenGl::GenerateMipmapNE(GLenum target) noexcept
{
    glGenerateMipmap(target);
}

void OpenGl::GenerateMipmap(GLenum target)
{
    GenerateMipmapNE(target);
    Internal::Check("glGenerateMipmap(target: {})", target);
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
