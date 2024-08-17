#pragma once

#include <fmt/core.h>

#include <memory>
#include <optional>
#include <span>

#include "klgl/formatting/simple_format.hpp"
#include "klgl/opengl/gl_api.hpp"

namespace klgl
{

struct TextureFormatInfo
{
    uint8_t n : 3 = 0;  // num_components
    uint8_t r : 6 = 0;  // num bits in red component
    uint8_t g : 6 = 0;  // num bits in green component
    uint8_t b : 6 = 0;  // num bits in blue component
    uint8_t a : 6 = 0;  // num bits in alpha component
    bool base : 1 = 0;  // Is base internal format. Like GL_RGBA, GL_RG, etc.
    bool norm : 1 = 0;  // is normalized
    bool sign : 1 = 0;  // is signed
    bool fp : 1 = 0;    // is floating point
};

struct PixelBufferFormatInfo
{
    size_t num_channels = 0;
    size_t bits_per_channel = 0;
};

struct TextureFormatHelper
{
    static std::optional<PixelBufferFormatInfo>
    TryGetPixelBufferFormatInfo(const GLint channels_format, const GLenum channel_type, std::string* out_error);
    static std::optional<size_t> TryGetPixelBufferChannelsCount(const GLint format, std::string* out_error);
    static std::optional<size_t> TryGetPixelBufferChannelSize(const GLenum channel_type, std::string* out_error);
    static std::optional<TextureFormatInfo> TryGetTextureInternalFormatInfo(
        const GLint internal_format,
        std::string* out_error);
    static TextureFormatInfo GetTextureInternalFormatInfo(const GLint internal_format);
};

struct PixelBufferFormat
{
    // The number of channels in image format
    std::optional<size_t> TryGetNumChannels(std::string* out_error) const;
    size_t GetNumChannels() const;

    // The number of bits in one channel for this format
    std::optional<size_t> TryGetChannelSize(std::string* out_error) const;
    size_t GetChannelSize() const;

    void ValidateBufferSize(const Vec2<size_t>& resolution, const size_t num_bytes) const;
    void EnsureCompatibleWithInternalTextureFormat(const GLint internal_texture_format) const;

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
    TextureFormatInfo GetFormatInfo() const { return TextureFormatHelper::GetTextureInternalFormatInfo(format_); }

    static std::unique_ptr<Texture> CreateEmpty(const Vec2<size_t>& resolution, const GLint format = GL_RGBA8);

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
    GLint format_ = GL_RGBA;
};

}  // namespace klgl

KLGL_MAKE_STRUCT_FORMATTER(klgl::PixelBufferFormat, FormatTo);
