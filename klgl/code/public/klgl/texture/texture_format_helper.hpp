#pragma once

#include <ass/enum_map.hpp>

#include "EverydayTools/Math/Matrix.hpp"
#include "klgl/formatting/simple_format.hpp"
#include "klgl/opengl/enums.hpp"

namespace klgl::detail
{
template <typename T>
struct ChannelTypeTraits
{
    static constexpr bool Valid = false;
};

template <>
struct ChannelTypeTraits<GLfloat>
{
    static constexpr bool Valid = true;
    static constexpr auto Enum = GlPixelBufferChannelType::Float;
};

template <>
struct ChannelTypeTraits<GLubyte>
{
    static constexpr bool Valid = true;
    static constexpr auto Enum = GlPixelBufferChannelType::UByte;
};

template <typename T>
concept ChannelType = ChannelTypeTraits<T>::Valid;

}  // namespace klgl::detail

namespace klgl
{
using namespace edt::lazy_matrix_aliases;  // NOLINT

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

struct TextureFormatHelper
{
    static std::optional<size_t> TryGetPixelBufferChannelsCount(
        const GlPixelBufferLayout layout,
        std::string* out_error);
    static size_t GetPixelBufferChannelsCount(const GlPixelBufferLayout layout);

    static std::optional<size_t> TryGetPixelBufferChannelSize(
        const GlPixelBufferChannelType type,
        std::string* out_error);
    static size_t GetPixelBufferChannelSize(const GlPixelBufferChannelType type);

    static std::optional<TextureFormatInfo> TryGetTextureInternalFormatInfo(
        const GlTextureInternalFormat format,
        std::string* out_error);
    static TextureFormatInfo GetTextureInternalFormatInfo(const GlTextureInternalFormat format);
};

struct PixelBufferFormat
{
    void ValidateBufferSize(const Vec2<size_t>& resolution, const size_t num_bytes) const;
    void EnsureCompatibleWithInternalTextureFormat(const GlTextureInternalFormat format) const;

    template <typename FmtContext>
    constexpr auto FormatTo(FmtContext& ctx) const
    {
        return format_to(ctx.out(), "{}[{}]", layout, type);
    }

    GlPixelBufferLayout layout = GlPixelBufferLayout::RGB;
    GlPixelBufferChannelType type = GlPixelBufferChannelType::UByte;
};
}  // namespace klgl

KLGL_MAKE_STRUCT_FORMATTER(klgl::PixelBufferFormat, FormatTo);
