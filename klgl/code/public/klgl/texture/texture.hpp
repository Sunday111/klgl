#pragma once

#include <memory>
#include <optional>
#include <span>

#include "klgl/opengl/detail/maps/gl_pixel_buffer_layout_to_num_channels.hpp"
#include "klgl/texture/texture_format_helper.hpp"

namespace klgl
{

class Texture
{
public:
    TextureFormatInfo GetFormatInfo() const { return TextureFormatHelper::GetTextureInternalFormatInfo(format_); }

    static std::unique_ptr<Texture> CreateEmpty(
        const Vec2<size_t>& resolution,
        const GlTextureInternalFormat format = GlTextureInternalFormat::RGBA8);

    ~Texture();

    void Bind() const;

    template <GlPixelBufferLayout layout, detail::ChannelType T, size_t extent = std::dynamic_extent>
    void SetPixels(
        std::span<const edt::Matrix<T, detail::kPixelBufferLayoutToNumChannels.Get(layout), 1>, extent> pixel_data)
    {
        SetPixels(
            {
                .layout = layout,
                .type = detail::ChannelTypeTraits<T>::Enum,
            },
            std::span(reinterpret_cast<const uint8_t*>(pixel_data.data()), pixel_data.size_bytes())  // NOLINT
        );
    }

    template <GlPixelBufferLayout layout, detail::ChannelType T, size_t extent = std::dynamic_extent>
        requires(detail::kPixelBufferLayoutToNumChannels.Get(layout) == 1)
    void SetPixels(std::span<const T, extent> pixel_data)
    {
        SetPixels(
            {
                .layout = layout,
                .type = detail::ChannelTypeTraits<T>::Enum,
            },
            std::span(reinterpret_cast<const uint8_t*>(pixel_data.data()), pixel_data.size_bytes())  // NOLINT
        );
    }

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
    GlTextureInternalFormat format_ = GlTextureInternalFormat::RGBA8;
};

}  // namespace klgl
