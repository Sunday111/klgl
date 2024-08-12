#pragma once

#include <fmt/core.h>

#include <memory>
#include <optional>
#include <span>

#include "klgl/formatting/simple_format.hpp"
#include "klgl/opengl/gl_api.hpp"

namespace klgl
{

struct PixelBufferFormat
{
    constexpr bool operator<=>(const PixelBufferFormat&) const = default;

    std::optional<size_t> TryGetNumChannels(std::string* out_error) const;
    size_t GetNumChannels() const;

    std::optional<size_t> TryGetChannelSize(std::string* out_error) const;
    size_t GetChannelSize() const;

    void ValidateBufferSize(const Vec2<size_t>& resolution, const size_t num_bytes) const;

    template <typename FmtContext>
    constexpr auto FormatTo(FmtContext& ctx) const
    {
        return format_to(
            ctx.out(),
            "{}[{}]",
            klgl::OpenGl::ToString(channels_format),
            klgl::OpenGl::ToString(channel_type));
    }

    GLint channels_format = GL_RGB;
    GLenum channel_type = GL_UNSIGNED_BYTE;
};

class Texture
{
public:
    static std::unique_ptr<Texture> CreateEmpty(const Vec2<size_t>& resolution, const PixelBufferFormat& format);

    ~Texture();

    void Bind() const;

    void SetPixelsRGB(std::span<const Vec3<uint8_t>> pixel_data);
    void SetPixelsRGBA(std::span<const Vec4<uint8_t>> pixel_data);

    Vec2<size_t> GetSize() const { return resolution_; }
    size_t GetWidth() const { return GetSize().x(); }
    size_t GetHeight() const { return GetSize().y(); }
    std::optional<GLuint> GetTexture() const { return texture_; }

private:
    void SetPixels(const PixelBufferFormat& format, std::span<const uint8_t> data);

private:
    std::optional<GLuint> texture_;
    Vec2<size_t> resolution_;
    GLenum type_ = GL_TEXTURE_2D;
    PixelBufferFormat format_{};
};

}  // namespace klgl

KLGL_MAKE_STRUCT_FORMATTER(klgl::PixelBufferFormat, FormatTo);
