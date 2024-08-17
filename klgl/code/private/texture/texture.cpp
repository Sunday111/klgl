#include "klgl/texture/texture.hpp"

#include <ankerl/unordered_dense.h>
#include <fmt/format.h>

#include <cassert>

#include "ass/fixed_unordered_map.hpp"
#include "klgl/error_handling.hpp"

namespace klgl
{

using GlIntHasher = decltype([](const GLint v) { return std::bit_cast<size_t>(static_cast<int64_t>(v)); });
using GlEnumHasher = decltype([](const GLenum v) -> size_t { return v; });

inline constexpr auto kTextureBufferFormatToNumChannels = []
{
    ass::FixedUnorderedMap<14, GLint, uint8_t, GlIntHasher> map;

    auto add = [&](GLint key, auto value)
    {
        assert(!map.Contains(key));
        [[maybe_unused]] const bool success = map.TryAdd(key, value);
        assert(success);
    };

    add(GL_RED, 1);
    add(GL_RED_INTEGER, 1);
    add(GL_DEPTH_COMPONENT, 1);

    add(GL_RG, 2);
    add(GL_RG_INTEGER, 2);

    add(GL_RGB, 3);
    add(GL_RGB_INTEGER, 3);
    add(GL_BGR, 3);
    add(GL_BGR_INTEGER, 3);

    add(GL_RGBA, 4);
    add(GL_RGBA_INTEGER, 4);
    add(GL_BGRA, 4);
    add(GL_BGRA_INTEGER, 4);

    add(GL_DEPTH_STENCIL, 2);

    return map;
}();

inline constexpr auto kTextureChannelTypeToBitsCount = []
{
    ass::FixedUnorderedMap<14, GLenum, uint8_t, GlEnumHasher> map;

    auto add = [&](const GLenum key, const uint8_t value)
    {
        assert(!map.Contains(key));
        map.Add(key, value);
    };

    add(GL_UNSIGNED_BYTE, 8);
    add(GL_FLOAT, 32);

    return map;
}();

inline constexpr auto kTextureInteralFormatToFormatInfo = []
{
    ass::FixedUnorderedMap<67, GLint, TextureFormatInfo, GlIntHasher> map;

    auto add = [&]<typename... Args>(const GLint key, TextureFormatInfo value)
    {
        assert(!map.Contains(key));
        map.Add(key, value);
    };

    // Base internal formats
    // See table 1 at https://registry.khronos.org/OpenGL-Refpages/gl4/html/glTexImage2D.xhtml
    add(GL_DEPTH_COMPONENT, {.n = 1, .base = 1});
    add(GL_DEPTH_STENCIL, {.n = 2, .base = 1});
    add(GL_RED, {.n = 1, .base = 1});
    add(GL_RG, {.n = 2, .base = 1});
    add(GL_RGB, {.n = 3, .base = 1});
    add(GL_RGBA, {.n = 4, .base = 1});

    // Sized internal formats
    // See table 2 at https://registry.khronos.org/OpenGL-Refpages/gl4/html/glTexImage2D.xhtml
    add(GL_R8, {.n = 1, .r = 8, .norm = 1, .sign = 0, .fp = 0});
    add(GL_R8_SNORM, {.n = 1, .r = 8, .norm = 1, .sign = 1, .fp = 0});
    add(GL_R16, {.n = 1, .r = 16, .norm = 1, .sign = 0, .fp = 0});
    add(GL_R16_SNORM, {.n = 1, .r = 16, .norm = 1, .sign = 1, .fp = 0});
    add(GL_RG8, {.n = 2, .r = 8, .g = 8, .norm = 1, .sign = 0, .fp = 0});
    add(GL_RG8_SNORM, {.n = 2, .r = 8, .g = 8, .norm = 1, .sign = 1, .fp = 0});
    add(GL_RG16, {.n = 2, .r = 16, .b = 16, .norm = 1, .sign = 0, .fp = 0});
    add(GL_RG16_SNORM, {.n = 2, .r = 16, .b = 16, .norm = 1, .sign = 1, .fp = 0});
    add(GL_R3_G3_B2, {.n = 3, .r = 3, .g = 3, .b = 2, .norm = 1, .sign = 0, .fp = 0});
    add(GL_RGB4, {.n = 3, .r = 4, .g = 4, .b = 4, .norm = 1, .sign = 0, .fp = 0});
    add(GL_RGB5, {.n = 3, .r = 5, .g = 5, .b = 5, .norm = 1, .sign = 0, .fp = 0});
    add(GL_RGB8, {.n = 3, .r = 8, .g = 8, .b = 8, .norm = 1, .sign = 0, .fp = 0});
    add(GL_RGB8_SNORM, {.n = 3, .r = 8, .g = 8, .b = 8, .norm = 1, .sign = 1, .fp = 0});
    add(GL_RGB10, {.n = 3, .r = 10, .g = 10, .b = 10, .norm = 1, .sign = 0, .fp = 0});
    add(GL_RGB12, {.n = 3, .r = 12, .g = 12, .b = 12, .norm = 1, .sign = 0, .fp = 0});
    add(GL_RGB16_SNORM, {.n = 3, .r = 16, .g = 16, .b = 16, .norm = 1, .sign = 1, .fp = 0});
    add(GL_RGBA2, {.n = 4, .r = 2, .g = 2, .b = 2, .a = 2, .norm = 1, .sign = 0, .fp = 0});
    add(GL_RGBA4, {.n = 4, .r = 4, .g = 4, .b = 4, .a = 4, .norm = 1, .sign = 0, .fp = 0});
    add(GL_RGB5_A1, {.n = 4, .r = 5, .g = 5, .b = 5, .a = 1, .norm = 1, .sign = 0, .fp = 0});
    add(GL_RGBA8, {.n = 4, .r = 8, .g = 8, .b = 8, .a = 8, .norm = 1, .sign = 0, .fp = 0});
    add(GL_RGBA8_SNORM, {.n = 4, .r = 8, .g = 8, .b = 8, .a = 8, .norm = 1, .sign = 1, .fp = 0});
    add(GL_RGB10_A2, {.n = 4, .r = 10, .g = 10, .b = 10, .a = 2, .norm = 1, .sign = 0, .fp = 0});
    add(GL_RGB10_A2UI, {.n = 4, .r = 10, .g = 10, .b = 10, .a = 2, .norm = 0, .sign = 0, .fp = 0});
    add(GL_RGBA12, {.n = 4, .r = 12, .g = 12, .b = 12, .a = 12, .norm = 1, .sign = 0, .fp = 0});
    add(GL_RGBA16, {.n = 4, .r = 16, .g = 16, .b = 16, .a = 16, .norm = 1, .sign = 0, .fp = 0});
    add(GL_SRGB8, {.n = 3, .r = 8, .g = 8, .b = 8, .norm = 1, .sign = 0, .fp = 0});
    add(GL_SRGB8_ALPHA8, {.n = 4, .r = 8, .g = 8, .b = 8, .a = 8, .norm = 1, .sign = 0, .fp = 0});
    add(GL_R16F, {.n = 1, .r = 16, .norm = 0, .sign = 1, .fp = 1});
    add(GL_RG16F, {.n = 2, .r = 16, .g = 16, .norm = 0, .sign = 1, .fp = 1});
    add(GL_RGB16F, {.n = 3, .r = 16, .g = 16, .b = 16, .norm = 0, .sign = 1, .fp = 1});
    add(GL_RGBA16F, {.n = 4, .r = 16, .g = 16, .b = 16, .a = 16, .norm = 0, .sign = 1, .fp = 1});
    add(GL_R32F, {.n = 1, .r = 32, .norm = 0, .sign = 1, .fp = 1});
    add(GL_RG32F, {.n = 2, .r = 32, .g = 32, .norm = 0, .sign = 1, .fp = 1});
    add(GL_RGB32F, {.n = 3, .r = 32, .g = 32, .b = 32, .norm = 0, .sign = 1, .fp = 1});
    add(GL_RGBA32F, {.n = 4, .r = 32, .g = 32, .b = 32, .a = 32, .norm = 0, .sign = 1, .fp = 1});
    add(GL_R11F_G11F_B10F, {.n = 3, .r = 11, .g = 11, .b = 10, .norm = 0, .sign = 1, .fp = 1});
    add(GL_RGB9_E5, {.n = 3, .r = 9, .g = 9, .b = 9, .a = 5, .norm = 0, .sign = 1, .fp = 1});
    add(GL_R8I, {.n = 1, .r = 8, .norm = 0, .sign = 1, .fp = 0});
    add(GL_R8UI, {.n = 1, .r = 8, .norm = 0, .sign = 0, .fp = 0});
    add(GL_R16I, {.n = 1, .r = 16, .norm = 0, .sign = 1, .fp = 0});
    add(GL_R16UI, {.n = 1, .r = 16, .norm = 0, .sign = 0, .fp = 0});
    add(GL_R32I, {.n = 1, .r = 32, .norm = 0, .sign = 1, .fp = 0});
    add(GL_R32UI, {.n = 1, .r = 32, .norm = 0, .sign = 0, .fp = 0});
    add(GL_RG8I, {.n = 2, .r = 8, .g = 8, .norm = 0, .sign = 1, .fp = 0});
    add(GL_RG8UI, {.n = 2, .r = 8, .g = 8, .norm = 0, .sign = 0, .fp = 0});
    add(GL_RG16I, {.n = 2, .r = 16, .g = 8, .norm = 0, .sign = 1, .fp = 0});
    add(GL_RG16UI, {.n = 2, .r = 16, .g = 8, .norm = 0, .sign = 0, .fp = 0});
    add(GL_RG32I, {.n = 2, .r = 32, .g = 8, .norm = 0, .sign = 1, .fp = 0});
    add(GL_RG32UI, {.n = 2, .r = 32, .g = 8, .norm = 0, .sign = 0, .fp = 0});
    add(GL_RGB8I, {.n = 3, .r = 8, .g = 8, .b = 8, .norm = 0, .sign = 1, .fp = 0});
    add(GL_RGB8UI, {.n = 3, .r = 8, .g = 8, .b = 8, .norm = 0, .sign = 0, .fp = 0});
    add(GL_RGB16I, {.n = 3, .r = 16, .g = 8, .b = 8, .norm = 0, .sign = 1, .fp = 0});
    add(GL_RGB16UI, {.n = 3, .r = 16, .g = 8, .b = 8, .norm = 0, .sign = 0, .fp = 0});
    add(GL_RGB32I, {.n = 3, .r = 32, .g = 8, .b = 8, .norm = 0, .sign = 1, .fp = 0});
    add(GL_RGB32UI, {.n = 3, .r = 32, .g = 8, .b = 8, .norm = 0, .sign = 0, .fp = 0});
    add(GL_RGBA8I, {.n = 4, .r = 8, .g = 8, .b = 8, .a = 8, .norm = 0, .sign = 1, .fp = 0});
    add(GL_RGBA8UI, {.n = 4, .r = 8, .g = 8, .b = 8, .a = 8, .norm = 0, .sign = 0, .fp = 0});
    add(GL_RGBA16I, {.n = 4, .r = 16, .g = 8, .b = 8, .a = 8, .norm = 0, .sign = 1, .fp = 0});
    add(GL_RGBA16UI, {.n = 4, .r = 16, .g = 8, .b = 8, .a = 8, .norm = 0, .sign = 0, .fp = 0});
    add(GL_RGBA32I, {.n = 4, .r = 32, .g = 8, .b = 8, .a = 8, .norm = 0, .sign = 1, .fp = 0});
    add(GL_RGBA32UI, {.n = 4, .r = 32, .g = 8, .b = 8, .a = 8, .norm = 0, .sign = 0, .fp = 0});

    // Maybe worth adding compressed formats, idk

    return map;
}();

std::optional<TextureFormatInfo> TextureFormatHelper::TryGetTextureInternalFormatInfo(
    const GLint internal_format,
    std::string* out_error)
{
    if (!kTextureInteralFormatToFormatInfo.Contains(internal_format))
    {
        if (out_error)
        {
            fmt::format_to(std::back_inserter(*out_error), "Unsupported internal format {}", internal_format);
        }
        return std::nullopt;
    }

    return kTextureInteralFormatToFormatInfo.Get(internal_format);
}

TextureFormatInfo TextureFormatHelper::GetTextureInternalFormatInfo(const GLint internal_format)
{
    std::string error;
    auto opt = TryGetTextureInternalFormatInfo(internal_format, &error);
    klgl::ErrorHandling::Ensure(opt.has_value(), "{}", error);
    return *opt;
}

std::optional<PixelBufferFormatInfo> TextureFormatHelper::TryGetPixelBufferFormatInfo(
    const GLint channels_format,
    const GLenum channel_type,
    std::string* out_error)
{
    size_t num_channels = 0;
    size_t bits_per_channel = 0;
    if (!TryGetPixelBufferChannelsCount(channels_format, out_error))
    {
        return std::nullopt;
    }

    if (!TryGetPixelBufferChannelSize(channel_type, out_error))
    {
        return std::nullopt;
    }

    return PixelBufferFormatInfo{
        .num_channels = num_channels,
        .bits_per_channel = bits_per_channel,
    };
}

std::optional<size_t> TextureFormatHelper::TryGetPixelBufferChannelsCount(const GLint format, std::string* out_error)
{
    if (!kTextureBufferFormatToNumChannels.Contains(format))
    {
        if (out_error)
        {
            fmt::format_to(std::back_inserter(*out_error), "Unsupported pixel image format {}", format);
        }
        return std::nullopt;
    }

    return kTextureBufferFormatToNumChannels.Get(format);
}

std::optional<size_t> PixelBufferFormat::TryGetNumChannels(std::string* out_error) const
{
    return TextureFormatHelper::TryGetPixelBufferChannelsCount(channels_format, out_error);
}

size_t PixelBufferFormat::GetNumChannels() const
{
    std::string error;
    auto opt = TryGetNumChannels(&error);
    klgl::ErrorHandling::Ensure(opt.has_value(), "{}", error);
    return *opt;
}

std::optional<size_t> TextureFormatHelper::TryGetPixelBufferChannelSize(
    const GLenum channel_type,
    std::string* out_error)
{
    if (!kTextureChannelTypeToBitsCount.Contains(channel_type))
    {
        if (out_error)
        {
            fmt::format_to(std::back_inserter(*out_error), "Unsupported channel type {}", channel_type);
        }
        return std::nullopt;
    }

    return kTextureChannelTypeToBitsCount.Get(channel_type);
}

std::optional<size_t> PixelBufferFormat::TryGetChannelSize(std::string* out_error) const
{
    switch (channel_type)
    {
    case GL_UNSIGNED_BYTE:
        return 1;
        break;
    case GL_FLOAT:
        return 4;
        break;
    default:
        if (out_error)
        {
            fmt::format_to(std::back_inserter(*out_error), "Unsupported pixel data type {}", channel_type);
        }
        return std::nullopt;
        break;
    }
}

size_t PixelBufferFormat::GetChannelSize() const
{
    std::string error;
    auto opt = TryGetChannelSize(&error);
    klgl::ErrorHandling::Ensure(opt.has_value(), "{}", error);
    return *opt;
}

void PixelBufferFormat::ValidateBufferSize(const Vec2<size_t>& resolution, const size_t num_bytes) const
{
    const size_t num_channels = GetNumChannels();
    const size_t channel_size = GetChannelSize();
    const size_t texel_size = num_channels * channel_size;
    const size_t expected_buffer_size = texel_size * resolution.x() * resolution.y();
    klgl::ErrorHandling::Ensure(
        expected_buffer_size == num_bytes,
        "Invalid buffer size for {}x{} texture with pixel of size {} ({} channels). "
        "Expected buffer with {} bytes but input buffer has {} bytes",
        resolution.x(),
        resolution.y(),
        texel_size,
        num_channels,
        expected_buffer_size,
        num_bytes);
}

void PixelBufferFormat::EnsureCompatibleWithInternalTextureFormat(const GLint internal_texture_format) const
{
    const size_t num_channels = GetNumChannels();
    // const size_t channel_size = GetChannelSize();
    const TextureFormatInfo texture_format_info =
        TextureFormatHelper::GetTextureInternalFormatInfo(internal_texture_format);

    klgl::ErrorHandling::Ensure(
        num_channels == texture_format_info.n,
        "Attempt to set pixel data of texture with {} channels providing data with {} channels only",
        texture_format_info.n,
        num_channels);
}

std::unique_ptr<Texture> Texture::CreateEmpty(const Vec2<size_t>& resolution, const GLint format)
{
    ErrorHandling::Ensure(resolution != Vec2<size_t>{}, "Empty texture size!");

    const TextureFormatInfo texture_format_info = TextureFormatHelper::GetTextureInternalFormatInfo(format);
    klgl::ErrorHandling::Ensure(!texture_format_info.base, "Do not use base internal formats.");

    auto tex = std::make_unique<Texture>();
    tex->texture_ = OpenGl::GenTexture();
    tex->type_ = GL_TEXTURE_2D;
    tex->resolution_ = resolution;
    tex->format_ = format;
    tex->Bind();

    assert(tex->type_ == GL_TEXTURE_2D);
    // auto err = glGetError();
    // assert(err == GL_NO_ERROR);
    OpenGl::TexImage2d(tex->type_, 0, format, resolution.x(), resolution.y(), GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    // err = glGetError();
    // assert(err == GL_NO_ERROR);

    OpenGl::SetTexture2dWrap(GlTextureWrap::R, GlTextureWrapMode::Repeat);
    OpenGl::SetTexture2dWrap(GlTextureWrap::S, GlTextureWrapMode::Repeat);
    OpenGl::SetTexture2dWrap(GlTextureWrap::T, GlTextureWrapMode::Repeat);
    OpenGl::SetTexture2dMagFilter(GlTextureFilter::Nearest);
    OpenGl::SetTexture2dMinFilter(GlTextureFilter::Nearest);

    return tex;
}

void Texture::Bind() const
{
    OpenGl::BindTexture(type_, *texture_);
}

void Texture::SetPixels(const PixelBufferFormat& format, std::span<const uint8_t> data)
{
    format.ValidateBufferSize(resolution_, data.size_bytes());
    format.EnsureCompatibleWithInternalTextureFormat(format_);

    assert(type_ == GL_TEXTURE_2D);
    Bind();
    constexpr GLint x_offset = 0, y_offset = 0;
    glTexSubImage2D(
        type_,
        0,
        x_offset,
        y_offset,
        static_cast<GLsizei>(resolution_.x()),
        static_cast<GLsizei>(resolution_.y()),
        format.channels_format,
        format.channel_type,
        data.data());
    assert(glGetError() == GL_NO_ERROR);

    // std::vector<Vec3<uint8_t>> got_pixels;
    // got_pixels.resize(p.pixel_data.size());
    // glGetTexImage(GL_TEXTURE_2D, 0, p.pixel_data_format, p.pixel_data_type, got_pixels.data());
    // [[maybe_unused]] auto err = glGetError();
    // assert(err == GL_NO_ERROR);
    //
    // std::vector<size_t> different_indices;
    // for (size_t i = 0; i != got_pixels.size(); ++i)
    // {
    //     if (p.pixel_data[i] != got_pixels[i])
    //     {
    //         different_indices.push_back(i);
    //     }
    // }
    // assert(different_indices.empty());
}

void Texture::SetPixelsRGB(std::span<const Vec3<uint8_t>> pixel_data)
{
    SetPixels(
        {
            .channels_format = GL_RGB,
            .channel_type = GL_UNSIGNED_BYTE,
        },
        std::span(reinterpret_cast<const uint8_t*>(pixel_data.data()), pixel_data.size_bytes())  // NOLINT
    );
}

void Texture::SetPixelsRGBA(std::span<const Vec4<uint8_t>> pixel_data)
{
    SetPixels(
        {
            .channels_format = GL_RGBA,
            .channel_type = GL_UNSIGNED_BYTE,
        },
        std::span(reinterpret_cast<const uint8_t*>(pixel_data.data()), pixel_data.size_bytes())  // NOLINT
    );
}

Texture::~Texture()
{
    if (texture_)
    {
        glDeleteTextures(1, &*texture_);
    }
}

}  // namespace klgl
