#include "klgl/texture/texture.hpp"

#include <ankerl/unordered_dense.h>
#include <fmt/format.h>

#include <cassert>

#include "ass/fixed_unordered_map.hpp"
#include "klgl/error_handling.hpp"

namespace klgl
{

constexpr auto MakeFormatToNumChannelsInfo()
{
    ass::FixedUnorderedMap<14, GLint, uint8_t, decltype([](const GLint v) { return static_cast<size_t>(v); })> m;

    auto add = [&](GLint key, auto value)
    {
        assert(!m.Contains(key));
        [[maybe_unused]] const bool success = m.TryAdd(key, value);
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

    return m;
}

inline constexpr auto kFormatToNumChannels = MakeFormatToNumChannelsInfo();

std::optional<size_t> PixelBufferFormat::TryGetNumChannels(std::string* out_error) const
{
    if (!kFormatToNumChannels.Contains(channels_format))
    {
        if (out_error)
        {
            fmt::format_to(std::back_inserter(*out_error), "Unsupported pixel data format {}", channels_format);
        }
        return std::nullopt;
    }

    return kFormatToNumChannels.Get(channels_format);
}

size_t PixelBufferFormat::GetNumChannels() const
{
    std::string error;
    auto opt = TryGetNumChannels(&error);
    klgl::ErrorHandling::Ensure(opt.has_value(), "{}", error);
    return *opt;
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

std::unique_ptr<Texture> Texture::CreateEmpty(const Vec2<size_t>& resolution, const PixelBufferFormat& format)
{
    ErrorHandling::Ensure(resolution != Vec2<size_t>{}, "Empty texture size!");

    auto tex = std::make_unique<Texture>();
    tex->texture_ = OpenGl::GenTexture();
    tex->type_ = GL_TEXTURE_2D;
    tex->resolution_ = resolution;
    tex->format_ = format;
    tex->Bind();

    assert(tex->type_ == GL_TEXTURE_2D);
    // auto err = glGetError();
    // assert(err == GL_NO_ERROR);
    OpenGl::TexImage2d(
        tex->type_,
        0,
        format.channels_format,
        resolution.x(),
        resolution.y(),
        format.channels_format,
        format.channel_type,
        nullptr);
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
    ErrorHandling::Ensure(format == format_, "Format {} does not match internal format {}", format, format_);
    format.ValidateBufferSize(resolution_, data.size_bytes());

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
