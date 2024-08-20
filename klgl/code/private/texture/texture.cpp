#include "klgl/texture/texture.hpp"

#include <cassert>

#include "klgl/error_handling.hpp"
#include "klgl/opengl/detail/maps/to_gl_value/pixel_buffer_channel_type.hpp"
#include "klgl/opengl/detail/maps/to_gl_value/pixel_buffer_layout.hpp"
#include "klgl/opengl/detail/maps/to_gl_value/texture_internal_format.hpp"
#include "klgl/opengl/gl_api.hpp"

namespace klgl
{

void PixelBufferFormat::ValidateBufferSize(const Vec2<size_t>& resolution, const size_t num_bytes) const
{
    const size_t num_channels = TextureFormatHelper::GetPixelBufferChannelsCount(layout);
    const size_t channel_size = TextureFormatHelper::GetPixelBufferChannelSize(type) / 8;
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

void PixelBufferFormat::EnsureCompatibleWithInternalTextureFormat(const GlTextureInternalFormat format) const
{
    const size_t num_channels = TextureFormatHelper::GetPixelBufferChannelsCount(layout);
    const TextureFormatInfo texture_format_info = TextureFormatHelper::GetTextureInternalFormatInfo(format);

    klgl::ErrorHandling::Ensure(
        num_channels == texture_format_info.n,
        "Attempt to set pixel data of texture with {} channels providing data with {} channels only",
        texture_format_info.n,
        num_channels);
}

std::unique_ptr<Texture> Texture::CreateEmpty(const Vec2<size_t>& resolution, const GlTextureInternalFormat format)
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
    OpenGl::TexImage2d(
        tex->type_,
        0,
        ToGlValue(format),
        resolution.x(),
        resolution.y(),
        GL_RGBA,
        GL_UNSIGNED_BYTE,
        nullptr);
    klgl::ErrorHandling::CheckOpenGlError("OpenGl::TexImage2d");

    OpenGl::SetTextureWrap(GlTextureParameterTarget::Texture2d, GlTextureWrapAxis::R, GlTextureWrapMode::Repeat);
    OpenGl::SetTextureWrap(GlTextureParameterTarget::Texture2d, GlTextureWrapAxis::S, GlTextureWrapMode::Repeat);
    OpenGl::SetTextureWrap(GlTextureParameterTarget::Texture2d, GlTextureWrapAxis::T, GlTextureWrapMode::Repeat);
    OpenGl::SetTextureMagFilter(GlTextureParameterTarget::Texture2d, GlTextureFilter::Nearest);
    OpenGl::SetTextureMinFilter(GlTextureParameterTarget::Texture2d, GlTextureFilter::Nearest);

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
        ToGlValue(format.layout),
        ToGlValue(format.type),
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

Texture::~Texture()
{
    if (texture_)
    {
        glDeleteTextures(1, &*texture_);
    }
}

}  // namespace klgl
