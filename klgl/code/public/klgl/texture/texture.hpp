#pragma once

#include <memory>
#include <span>

#include "klgl/opengl/detail/maps/gl_pixel_buffer_layout_to_num_channels.hpp"
#include "klgl/opengl/identifiers.hpp"
#include "klgl/opengl/object.hpp"
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
    static std::unique_ptr<Texture> CreateDepthStencil(const Vec2<size_t>& resolution);

    ~Texture();

    void Bind() const;

    template <
        GlPixelBufferLayout layout,
        detail::ChannelType T,
        size_t extent = std::dynamic_extent,
        size_t num_ch = detail::kPixelBufferLayoutToNumChannels.Get(layout)>
    void SetPixels(std::span<const edt::Matrix<T, num_ch, 1>, extent> pixel_data)
    {
        SetPixels(
            {
                .layout = layout,
                .type = detail::ChannelTypeTraits<T>::Enum,
            },
            std::span(reinterpret_cast<const uint8_t*>(pixel_data.data()), pixel_data.size_bytes())  // NOLINT
        );
    }

    template <
        GlPixelBufferLayout layout,
        detail::ChannelType T,
        size_t extent = std::dynamic_extent,
        size_t num_ch = detail::kPixelBufferLayoutToNumChannels.Get(layout)>
        requires(num_ch == 1)
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
    GlTextureId GetTexture() const { return texture_; }

private:
    void SetPixels(const PixelBufferFormat& format, std::span<const uint8_t> data);

private:
    GlObject<GlTextureId> texture_;
    Vec2<size_t> resolution_;
    GlTargetTextureType type_ = GlTargetTextureType::Texture2d;
    GlTextureInternalFormat format_ = GlTextureInternalFormat::RGBA8;
};

}  // namespace klgl
