#include "klgl/texture/texture.hpp"

#include <cassert>

#include "klgl/error_handling.hpp"

namespace klgl
{

std::unique_ptr<Texture> Texture::CreateEmpty(size_t width, size_t height, GLint internal_format)
{
    assert(width != 0 && height != 0);
    auto tex = std::make_unique<Texture>();
    tex->texture_ = OpenGl::GenTexture();
    tex->type_ = GL_TEXTURE_2D;
    tex->width_ = width;
    tex->height_ = height;
    tex->Bind();
    GLenum pixel_data_format = GL_RGB;
    GLenum pixel_data_type = GL_UNSIGNED_BYTE;

    assert(tex->type_ == GL_TEXTURE_2D);
    // auto err = glGetError();
    // assert(err == GL_NO_ERROR);
    OpenGl::TexImage2d(tex->type_, 0, internal_format, width, height, pixel_data_format, pixel_data_type, nullptr);
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

void Texture::SetPixelsImpl(const SetPixelsParameters& p)
{
    size_t num_channels = 0;
    switch (p.pixel_data_format)
    {
    case GL_RGB:
        num_channels = 3;
        break;
    case GL_RGBA:
        num_channels = 4;
        break;
    default:
        klgl::ErrorHandling::ThrowWithMessage("Unsupported pixel data format {}", p.pixel_data_format);
        break;
    }

    size_t channel_size = 0;
    switch (p.pixel_data_type)
    {
    case GL_UNSIGNED_BYTE:
        channel_size = 1;
        break;
    case GL_FLOAT:
        channel_size = 4;
        break;
    default:
        klgl::ErrorHandling::ThrowWithMessage("Unsupported pixel data type {}", p.pixel_data_type);
        break;
    }

    const size_t texel_size = num_channels * channel_size;
    const size_t expected_buffer_size = texel_size * width_ * height_;
    klgl::ErrorHandling::Ensure(
        expected_buffer_size == p.pixel_data.size_bytes(),
        "Invalid buffer size for {}x{} texture with pixel of size {} ({} channels). "
        "Expected buffer with {} bytes but input buffer has {} bytes",
        width_,
        height_,
        texel_size,
        num_channels,
        expected_buffer_size,
        p.pixel_data.size_bytes());

    assert(type_ == GL_TEXTURE_2D);
    Bind();
    constexpr GLint x_offset = 0, y_offset = 0;
    glTexSubImage2D(
        type_,
        0,
        x_offset,
        y_offset,
        static_cast<GLsizei>(width_),
        static_cast<GLsizei>(height_),
        p.pixel_data_format,
        p.pixel_data_type,
        p.pixel_data.data());
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

void Texture::SetPixels(std::span<const Vec3<uint8_t>> pixel_data)
{
    SetPixelsImpl({
        .pixel_data_format = GL_RGB,
        .pixel_data_type = GL_UNSIGNED_BYTE,
        .pixel_data =
            std::span(reinterpret_cast<const uint8_t*>(pixel_data.data()), pixel_data.size_bytes()),  // NOLINT
    });
}

void Texture::SetPixels(std::span<const Vec4<uint8_t>> pixel_data)
{
    SetPixelsImpl({
        .pixel_data_format = GL_RGBA,
        .pixel_data_type = GL_UNSIGNED_BYTE,
        .pixel_data =
            std::span(reinterpret_cast<const uint8_t*>(pixel_data.data()), pixel_data.size_bytes()),  // NOLINT
    });
}

Texture::~Texture()
{
    if (texture_)
    {
        glDeleteTextures(1, &*texture_);
    }
}

}  // namespace klgl
