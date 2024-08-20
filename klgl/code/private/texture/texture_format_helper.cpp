#include "klgl/texture/texture_format_helper.hpp"

#include <ass/enum_map.hpp>

#include "klgl/error_handling.hpp"
#include "klgl/opengl/detail/maps/gl_pixel_buffer_layout_to_num_channels.hpp"

namespace klgl
{

inline constexpr auto kTextureChannelTypeToBitsCount = []
{
    ass::EnumMap<GlPixelBufferChannelType, uint8_t> map;

    auto add = [&](auto key, uint8_t value)
    {
        assert(!map.Contains(key));
        map.GetOrAdd(key) = value;
    };

    KLGL_ENSURE_ENUM_SIZE(GlPixelBufferChannelType, 2);
    add(GlPixelBufferChannelType::UByte, 8);
    add(GlPixelBufferChannelType::Float, 32);

    return map;
}();

inline constexpr auto kTextureInteralFormatToFormatInfo = []
{
    ass::EnumMap<GlTextureInternalFormat, TextureFormatInfo> map;

    auto add = [&]<typename... Args>(const auto key, TextureFormatInfo value)
    {
        assert(!map.Contains(key));
        map.GetOrAdd(key) = value;
    };

    using T = GlTextureInternalFormat;

    KLGL_ENSURE_ENUM_SIZE(GlTextureInternalFormat, 67);

    // Base internal formats
    // See table 1 at https://registry.khronos.org/OpenGL-Refpages/gl4/html/glTexImage2D.xhtml
    add(T::DEPTH_COMPONENT, {.n = 1, .base = 1});
    add(T::DEPTH_STENCIL, {.n = 2, .base = 1});
    add(T::RED, {.n = 1, .base = 1});
    add(T::RG, {.n = 2, .base = 1});
    add(T::RGB, {.n = 3, .base = 1});
    add(T::RGBA, {.n = 4, .base = 1});

    // Sized internal formats
    // See table 2 at https://registry.khronos.org/OpenGL-Refpages/gl4/html/glTexImage2D.xhtml
    add(T::R8, {.n = 1, .r = 8, .norm = 1, .sign = 0, .fp = 0});
    add(T::R8_SNORM, {.n = 1, .r = 8, .norm = 1, .sign = 1, .fp = 0});
    add(T::R16, {.n = 1, .r = 16, .norm = 1, .sign = 0, .fp = 0});
    add(T::R16_SNORM, {.n = 1, .r = 16, .norm = 1, .sign = 1, .fp = 0});
    add(T::RG8, {.n = 2, .r = 8, .g = 8, .norm = 1, .sign = 0, .fp = 0});
    add(T::RG8_SNORM, {.n = 2, .r = 8, .g = 8, .norm = 1, .sign = 1, .fp = 0});
    add(T::RG16, {.n = 2, .r = 16, .b = 16, .norm = 1, .sign = 0, .fp = 0});
    add(T::RG16_SNORM, {.n = 2, .r = 16, .b = 16, .norm = 1, .sign = 1, .fp = 0});
    add(T::R3_G3_B2, {.n = 3, .r = 3, .g = 3, .b = 2, .norm = 1, .sign = 0, .fp = 0});
    add(T::RGB4, {.n = 3, .r = 4, .g = 4, .b = 4, .norm = 1, .sign = 0, .fp = 0});
    add(T::RGB5, {.n = 3, .r = 5, .g = 5, .b = 5, .norm = 1, .sign = 0, .fp = 0});
    add(T::RGB8, {.n = 3, .r = 8, .g = 8, .b = 8, .norm = 1, .sign = 0, .fp = 0});
    add(T::RGB8_SNORM, {.n = 3, .r = 8, .g = 8, .b = 8, .norm = 1, .sign = 1, .fp = 0});
    add(T::RGB10, {.n = 3, .r = 10, .g = 10, .b = 10, .norm = 1, .sign = 0, .fp = 0});
    add(T::RGB12, {.n = 3, .r = 12, .g = 12, .b = 12, .norm = 1, .sign = 0, .fp = 0});
    add(T::RGB16_SNORM, {.n = 3, .r = 16, .g = 16, .b = 16, .norm = 1, .sign = 1, .fp = 0});
    add(T::RGBA2, {.n = 4, .r = 2, .g = 2, .b = 2, .a = 2, .norm = 1, .sign = 0, .fp = 0});
    add(T::RGBA4, {.n = 4, .r = 4, .g = 4, .b = 4, .a = 4, .norm = 1, .sign = 0, .fp = 0});
    add(T::RGB5_A1, {.n = 4, .r = 5, .g = 5, .b = 5, .a = 1, .norm = 1, .sign = 0, .fp = 0});
    add(T::RGBA8, {.n = 4, .r = 8, .g = 8, .b = 8, .a = 8, .norm = 1, .sign = 0, .fp = 0});
    add(T::RGBA8_SNORM, {.n = 4, .r = 8, .g = 8, .b = 8, .a = 8, .norm = 1, .sign = 1, .fp = 0});
    add(T::RGB10_A2, {.n = 4, .r = 10, .g = 10, .b = 10, .a = 2, .norm = 1, .sign = 0, .fp = 0});
    add(T::RGB10_A2UI, {.n = 4, .r = 10, .g = 10, .b = 10, .a = 2, .norm = 0, .sign = 0, .fp = 0});
    add(T::RGBA12, {.n = 4, .r = 12, .g = 12, .b = 12, .a = 12, .norm = 1, .sign = 0, .fp = 0});
    add(T::RGBA16, {.n = 4, .r = 16, .g = 16, .b = 16, .a = 16, .norm = 1, .sign = 0, .fp = 0});
    add(T::SRGB8, {.n = 3, .r = 8, .g = 8, .b = 8, .norm = 1, .sign = 0, .fp = 0});
    add(T::SRGB8_ALPHA8, {.n = 4, .r = 8, .g = 8, .b = 8, .a = 8, .norm = 1, .sign = 0, .fp = 0});
    add(T::R16F, {.n = 1, .r = 16, .norm = 0, .sign = 1, .fp = 1});
    add(T::RG16F, {.n = 2, .r = 16, .g = 16, .norm = 0, .sign = 1, .fp = 1});
    add(T::RGB16F, {.n = 3, .r = 16, .g = 16, .b = 16, .norm = 0, .sign = 1, .fp = 1});
    add(T::RGBA16F, {.n = 4, .r = 16, .g = 16, .b = 16, .a = 16, .norm = 0, .sign = 1, .fp = 1});
    add(T::R32F, {.n = 1, .r = 32, .norm = 0, .sign = 1, .fp = 1});
    add(T::RG32F, {.n = 2, .r = 32, .g = 32, .norm = 0, .sign = 1, .fp = 1});
    add(T::RGB32F, {.n = 3, .r = 32, .g = 32, .b = 32, .norm = 0, .sign = 1, .fp = 1});
    add(T::RGBA32F, {.n = 4, .r = 32, .g = 32, .b = 32, .a = 32, .norm = 0, .sign = 1, .fp = 1});
    add(T::R11F_G11F_B10F, {.n = 3, .r = 11, .g = 11, .b = 10, .norm = 0, .sign = 1, .fp = 1});
    add(T::RGB9_E5, {.n = 3, .r = 9, .g = 9, .b = 9, .a = 5, .norm = 0, .sign = 1, .fp = 1});
    add(T::R8I, {.n = 1, .r = 8, .norm = 0, .sign = 1, .fp = 0});
    add(T::R8UI, {.n = 1, .r = 8, .norm = 0, .sign = 0, .fp = 0});
    add(T::R16I, {.n = 1, .r = 16, .norm = 0, .sign = 1, .fp = 0});
    add(T::R16UI, {.n = 1, .r = 16, .norm = 0, .sign = 0, .fp = 0});
    add(T::R32I, {.n = 1, .r = 32, .norm = 0, .sign = 1, .fp = 0});
    add(T::R32UI, {.n = 1, .r = 32, .norm = 0, .sign = 0, .fp = 0});
    add(T::RG8I, {.n = 2, .r = 8, .g = 8, .norm = 0, .sign = 1, .fp = 0});
    add(T::RG8UI, {.n = 2, .r = 8, .g = 8, .norm = 0, .sign = 0, .fp = 0});
    add(T::RG16I, {.n = 2, .r = 16, .g = 8, .norm = 0, .sign = 1, .fp = 0});
    add(T::RG16UI, {.n = 2, .r = 16, .g = 8, .norm = 0, .sign = 0, .fp = 0});
    add(T::RG32I, {.n = 2, .r = 32, .g = 8, .norm = 0, .sign = 1, .fp = 0});
    add(T::RG32UI, {.n = 2, .r = 32, .g = 8, .norm = 0, .sign = 0, .fp = 0});
    add(T::RGB8I, {.n = 3, .r = 8, .g = 8, .b = 8, .norm = 0, .sign = 1, .fp = 0});
    add(T::RGB8UI, {.n = 3, .r = 8, .g = 8, .b = 8, .norm = 0, .sign = 0, .fp = 0});
    add(T::RGB16I, {.n = 3, .r = 16, .g = 8, .b = 8, .norm = 0, .sign = 1, .fp = 0});
    add(T::RGB16UI, {.n = 3, .r = 16, .g = 8, .b = 8, .norm = 0, .sign = 0, .fp = 0});
    add(T::RGB32I, {.n = 3, .r = 32, .g = 8, .b = 8, .norm = 0, .sign = 1, .fp = 0});
    add(T::RGB32UI, {.n = 3, .r = 32, .g = 8, .b = 8, .norm = 0, .sign = 0, .fp = 0});
    add(T::RGBA8I, {.n = 4, .r = 8, .g = 8, .b = 8, .a = 8, .norm = 0, .sign = 1, .fp = 0});
    add(T::RGBA8UI, {.n = 4, .r = 8, .g = 8, .b = 8, .a = 8, .norm = 0, .sign = 0, .fp = 0});
    add(T::RGBA16I, {.n = 4, .r = 16, .g = 8, .b = 8, .a = 8, .norm = 0, .sign = 1, .fp = 0});
    add(T::RGBA16UI, {.n = 4, .r = 16, .g = 8, .b = 8, .a = 8, .norm = 0, .sign = 0, .fp = 0});
    add(T::RGBA32I, {.n = 4, .r = 32, .g = 8, .b = 8, .a = 8, .norm = 0, .sign = 1, .fp = 0});
    add(T::RGBA32UI, {.n = 4, .r = 32, .g = 8, .b = 8, .a = 8, .norm = 0, .sign = 0, .fp = 0});

    // Maybe worth adding compressed formats, idk

    return map;
}();

std::optional<TextureFormatInfo> TextureFormatHelper::TryGetTextureInternalFormatInfo(
    const GlTextureInternalFormat format,
    std::string* out_error)
{
    if (!kTextureInteralFormatToFormatInfo.Contains(format))
    {
        if (out_error)
        {
            fmt::format_to(std::back_inserter(*out_error), "Unsupported internal format {}", format);
        }
        return std::nullopt;
    }

    return kTextureInteralFormatToFormatInfo.Get(format);
}

TextureFormatInfo TextureFormatHelper::GetTextureInternalFormatInfo(const GlTextureInternalFormat format)
{
    std::string error;
    auto opt = TryGetTextureInternalFormatInfo(format, &error);
    klgl::ErrorHandling::Ensure(opt.has_value(), "{}", error);
    return *opt;
}

std::optional<size_t> TextureFormatHelper::TryGetPixelBufferChannelsCount(
    const GlPixelBufferLayout layout,
    std::string* out_error)
{
    if (!detail::kPixelBufferLayoutToNumChannels.Contains(layout))
    {
        if (out_error)
        {
            fmt::format_to(std::back_inserter(*out_error), "Unsupported pixel buffer layout {}", layout);
        }
        return std::nullopt;
    }

    return detail::kPixelBufferLayoutToNumChannels.Get(layout);
}

size_t TextureFormatHelper::GetPixelBufferChannelsCount(const GlPixelBufferLayout layout)
{
    std::string error;
    auto opt = TryGetPixelBufferChannelsCount(layout, &error);
    klgl::ErrorHandling::Ensure(opt.has_value(), "{}", error);
    return *opt;
}

std::optional<size_t> TextureFormatHelper::TryGetPixelBufferChannelSize(
    const GlPixelBufferChannelType type,
    std::string* out_error)
{
    if (!kTextureChannelTypeToBitsCount.Contains(type))
    {
        if (out_error)
        {
            fmt::format_to(std::back_inserter(*out_error), "Unsupported channel type {}", type);
        }
        return std::nullopt;
    }

    return kTextureChannelTypeToBitsCount.Get(type);
}

size_t TextureFormatHelper::GetPixelBufferChannelSize(const GlPixelBufferChannelType type)
{
    std::string error;
    auto opt = TryGetPixelBufferChannelSize(type, &error);
    klgl::ErrorHandling::Ensure(opt.has_value(), "{}", error);
    return *opt;
}
}  // namespace klgl
