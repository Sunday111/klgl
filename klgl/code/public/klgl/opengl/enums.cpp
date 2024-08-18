#include "enums.hpp"

#include <ass/enum_map.hpp>

#include "klgl/macro/ensure_enum_size.hpp"

namespace klgl
{

inline constexpr auto kGlPixelBufferLayoutToGLint = []
{
    ass::EnumMap<GlPixelBufferLayout, GLint> map;

    auto add = [&](auto key, auto value)
    {
        assert(!map.Contains(key));
        map.GetOrAdd(key) = value;
    };

    KLGL_ENSURE_ENUM_SIZE(GlPixelBufferLayout, 14);
    add(GlPixelBufferLayout::R, GL_RED);
    add(GlPixelBufferLayout::R_Int, GL_RED_INTEGER);
    add(GlPixelBufferLayout::Depth, GL_DEPTH_COMPONENT);
    add(GlPixelBufferLayout::RG, GL_RG);
    add(GlPixelBufferLayout::RG_Int, GL_RG_INTEGER);
    add(GlPixelBufferLayout::RGB, GL_RGB);
    add(GlPixelBufferLayout::RGB_Int, GL_RGB_INTEGER);
    add(GlPixelBufferLayout::BGR, GL_BGR);
    add(GlPixelBufferLayout::BGR_Int, GL_BGR_INTEGER);
    add(GlPixelBufferLayout::RGBA, GL_RGBA);
    add(GlPixelBufferLayout::RGBA_Int, GL_RGBA_INTEGER);
    add(GlPixelBufferLayout::BGRA, GL_BGRA);
    add(GlPixelBufferLayout::BGRA_Int, GL_BGRA_INTEGER);
    add(GlPixelBufferLayout::DepthStencil, GL_DEPTH_STENCIL);

    return map;
}();

inline constexpr auto kGlPixelBufferChannelTypeToGLenum = []
{
    ass::EnumMap<GlPixelBufferChannelType, GLenum> map;

    auto add = [&](auto key, GLenum value)
    {
        assert(!map.Contains(key));
        map.GetOrAdd(key) = value;
    };

    KLGL_ENSURE_ENUM_SIZE(GlPixelBufferChannelType, 2);
    add(GlPixelBufferChannelType::UByte, GL_UNSIGNED_BYTE);
    add(GlPixelBufferChannelType::Float, GL_FLOAT);

    return map;
}();

inline constexpr auto kGlTextureInternalFormatToGLint = []
{
    ass::EnumMap<GlTextureInternalFormat, GLint> map;

    auto add = [&](auto key, GLint value)
    {
        assert(!map.Contains(key));
        map.GetOrAdd(key) = value;
    };

    KLGL_ENSURE_ENUM_SIZE(GlTextureInternalFormat, 67);

    add(GlTextureInternalFormat::DEPTH_COMPONENT, GL_DEPTH_COMPONENT);
    add(GlTextureInternalFormat::DEPTH_STENCIL, GL_DEPTH_STENCIL);
    add(GlTextureInternalFormat::RED, GL_RED);
    add(GlTextureInternalFormat::RG, GL_RG);
    add(GlTextureInternalFormat::RGB, GL_RGB);
    add(GlTextureInternalFormat::RGBA, GL_RGBA);
    add(GlTextureInternalFormat::R8, GL_R8);
    add(GlTextureInternalFormat::R8_SNORM, GL_R8_SNORM);
    add(GlTextureInternalFormat::R16, GL_R16);
    add(GlTextureInternalFormat::R16_SNORM, GL_R16_SNORM);
    add(GlTextureInternalFormat::RG8, GL_RG8);
    add(GlTextureInternalFormat::RG8_SNORM, GL_RG8_SNORM);
    add(GlTextureInternalFormat::RG16, GL_RG16);
    add(GlTextureInternalFormat::RG16_SNORM, GL_RG16_SNORM);
    add(GlTextureInternalFormat::R3_G3_B2, GL_R3_G3_B2);
    add(GlTextureInternalFormat::RGB4, GL_RGB4);
    add(GlTextureInternalFormat::RGB5, GL_RGB5);
    add(GlTextureInternalFormat::RGB8, GL_RGB8);
    add(GlTextureInternalFormat::RGB8_SNORM, GL_RGB8_SNORM);
    add(GlTextureInternalFormat::RGB10, GL_RGB10);
    add(GlTextureInternalFormat::RGB12, GL_RGB12);
    add(GlTextureInternalFormat::RGB16_SNORM, GL_RGB16_SNORM);
    add(GlTextureInternalFormat::RGBA2, GL_RGBA2);
    add(GlTextureInternalFormat::RGBA4, GL_RGBA4);
    add(GlTextureInternalFormat::RGB5_A1, GL_RGB5_A1);
    add(GlTextureInternalFormat::RGBA8, GL_RGBA8);
    add(GlTextureInternalFormat::RGBA8_SNORM, GL_RGBA8_SNORM);
    add(GlTextureInternalFormat::RGB10_A2, GL_RGB10_A2);
    add(GlTextureInternalFormat::RGB10_A2UI, GL_RGB10_A2UI);
    add(GlTextureInternalFormat::RGBA12, GL_RGBA12);
    add(GlTextureInternalFormat::RGBA16, GL_RGBA16);
    add(GlTextureInternalFormat::SRGB8, GL_SRGB8);
    add(GlTextureInternalFormat::SRGB8_ALPHA8, GL_SRGB8_ALPHA8);
    add(GlTextureInternalFormat::R16F, GL_R16F);
    add(GlTextureInternalFormat::RG16F, GL_RG16F);
    add(GlTextureInternalFormat::RGB16F, GL_RGB16F);
    add(GlTextureInternalFormat::RGBA16F, GL_RGBA16F);
    add(GlTextureInternalFormat::R32F, GL_R32F);
    add(GlTextureInternalFormat::RG32F, GL_RG32F);
    add(GlTextureInternalFormat::RGB32F, GL_RGB32F);
    add(GlTextureInternalFormat::RGBA32F, GL_RGBA32F);
    add(GlTextureInternalFormat::R11F_G11F_B10F, GL_R11F_G11F_B10F);
    add(GlTextureInternalFormat::RGB9_E5, GL_RGB9_E5);
    add(GlTextureInternalFormat::R8I, GL_R8I);
    add(GlTextureInternalFormat::R8UI, GL_R8UI);
    add(GlTextureInternalFormat::R16I, GL_R16I);
    add(GlTextureInternalFormat::R16UI, GL_R16UI);
    add(GlTextureInternalFormat::R32I, GL_R32I);
    add(GlTextureInternalFormat::R32UI, GL_R32UI);
    add(GlTextureInternalFormat::RG8I, GL_RG8I);
    add(GlTextureInternalFormat::RG8UI, GL_RG8UI);
    add(GlTextureInternalFormat::RG16I, GL_RG16I);
    add(GlTextureInternalFormat::RG16UI, GL_RG16UI);
    add(GlTextureInternalFormat::RG32I, GL_RG32I);
    add(GlTextureInternalFormat::RG32UI, GL_RG32UI);
    add(GlTextureInternalFormat::RGB8I, GL_RGB8I);
    add(GlTextureInternalFormat::RGB8UI, GL_RGB8UI);
    add(GlTextureInternalFormat::RGB16I, GL_RGB16I);
    add(GlTextureInternalFormat::RGB16UI, GL_RGB16UI);
    add(GlTextureInternalFormat::RGB32I, GL_RGB32I);
    add(GlTextureInternalFormat::RGB32UI, GL_RGB32UI);
    add(GlTextureInternalFormat::RGBA8I, GL_RGBA8I);
    add(GlTextureInternalFormat::RGBA8UI, GL_RGBA8UI);
    add(GlTextureInternalFormat::RGBA16I, GL_RGBA16I);
    add(GlTextureInternalFormat::RGBA16UI, GL_RGBA16UI);
    add(GlTextureInternalFormat::RGBA32I, GL_RGBA32I);
    add(GlTextureInternalFormat::RGBA32UI, GL_RGBA32UI);

    return map;
}();

GLint ToGlValue(GlPixelBufferLayout v)
{
    return kGlPixelBufferLayoutToGLint.Get(v);
}
GLenum ToGlValue(GlPixelBufferChannelType v)
{
    return kGlPixelBufferChannelTypeToGLenum.Get(v);
}

GLint ToGlValue(GlTextureInternalFormat v)
{
    return kGlTextureInternalFormatToGLint.Get(v);
}
}  // namespace klgl

#undef KLGL_STRING_MAP_ADD
