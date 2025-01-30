#pragma once

#include "ass/enum_map.hpp"
#include "klgl/macro/ensure_enum_size.hpp"
#include "klgl/opengl/detail/maps/to_gl_value/opengl_value_converter.hpp"
#include "klgl/opengl/enums.hpp"

namespace klgl::detail
{
inline constexpr auto kGlTextureInternalFormatToGlValue = []
{
    using T = GlTextureInternalFormat;
    OpenGlValueConverter<T, GLint> c;

    KLGL_ENSURE_ENUM_SIZE(T, 73);
    c.Add(T::DEPTH_COMPONENT, GL_DEPTH_COMPONENT);
    c.Add(T::DEPTH_STENCIL, GL_DEPTH_STENCIL);
    c.Add(T::RED, GL_RED);
    c.Add(T::RG, GL_RG);
    c.Add(T::RGB, GL_RGB);
    c.Add(T::RGBA, GL_RGBA);
    c.Add(T::R8, GL_R8);
    c.Add(T::R8_SNORM, GL_R8_SNORM);
    c.Add(T::R16, GL_R16);
    c.Add(T::R16_SNORM, GL_R16_SNORM);
    c.Add(T::RG8, GL_RG8);
    c.Add(T::RG8_SNORM, GL_RG8_SNORM);
    c.Add(T::RG16, GL_RG16);
    c.Add(T::RG16_SNORM, GL_RG16_SNORM);
    c.Add(T::R3_G3_B2, GL_R3_G3_B2);
    c.Add(T::RGB4, GL_RGB4);
    c.Add(T::RGB5, GL_RGB5);
    c.Add(T::RGB8, GL_RGB8);
    c.Add(T::RGB8_SNORM, GL_RGB8_SNORM);
    c.Add(T::RGB10, GL_RGB10);
    c.Add(T::RGB12, GL_RGB12);
    c.Add(T::RGB16_SNORM, GL_RGB16_SNORM);
    c.Add(T::RGBA2, GL_RGBA2);
    c.Add(T::RGBA4, GL_RGBA4);
    c.Add(T::RGB5_A1, GL_RGB5_A1);
    c.Add(T::RGBA8, GL_RGBA8);
    c.Add(T::RGBA8_SNORM, GL_RGBA8_SNORM);
    c.Add(T::RGB10_A2, GL_RGB10_A2);
    c.Add(T::RGB10_A2UI, GL_RGB10_A2UI);
    c.Add(T::RGBA12, GL_RGBA12);
    c.Add(T::RGBA16, GL_RGBA16);
    c.Add(T::SRGB8, GL_SRGB8);
    c.Add(T::SRGB8_ALPHA8, GL_SRGB8_ALPHA8);
    c.Add(T::R16F, GL_R16F);
    c.Add(T::RG16F, GL_RG16F);
    c.Add(T::RGB16F, GL_RGB16F);
    c.Add(T::RGBA16F, GL_RGBA16F);
    c.Add(T::R32F, GL_R32F);
    c.Add(T::RG32F, GL_RG32F);
    c.Add(T::RGB32F, GL_RGB32F);
    c.Add(T::RGBA32F, GL_RGBA32F);
    c.Add(T::R11F_G11F_B10F, GL_R11F_G11F_B10F);
    c.Add(T::RGB9_E5, GL_RGB9_E5);
    c.Add(T::R8I, GL_R8I);
    c.Add(T::R8UI, GL_R8UI);
    c.Add(T::R16I, GL_R16I);
    c.Add(T::R16UI, GL_R16UI);
    c.Add(T::R32I, GL_R32I);
    c.Add(T::R32UI, GL_R32UI);
    c.Add(T::RG8I, GL_RG8I);
    c.Add(T::RG8UI, GL_RG8UI);
    c.Add(T::RG16I, GL_RG16I);
    c.Add(T::RG16UI, GL_RG16UI);
    c.Add(T::RG32I, GL_RG32I);
    c.Add(T::RG32UI, GL_RG32UI);
    c.Add(T::RGB8I, GL_RGB8I);
    c.Add(T::RGB8UI, GL_RGB8UI);
    c.Add(T::RGB16I, GL_RGB16I);
    c.Add(T::RGB16UI, GL_RGB16UI);
    c.Add(T::RGB32I, GL_RGB32I);
    c.Add(T::RGB32UI, GL_RGB32UI);
    c.Add(T::RGBA8I, GL_RGBA8I);
    c.Add(T::RGBA8UI, GL_RGBA8UI);
    c.Add(T::RGBA16I, GL_RGBA16I);
    c.Add(T::RGBA16UI, GL_RGBA16UI);
    c.Add(T::RGBA32I, GL_RGBA32I);
    c.Add(T::RGBA32UI, GL_RGBA32UI);
    c.Add(T::DEPTH16, GL_DEPTH_COMPONENT16);
    c.Add(T::DEPTH24, GL_DEPTH_COMPONENT24);
    c.Add(T::DEPTH32F, GL_DEPTH_COMPONENT32F);
    c.Add(T::DEPTH24_STENCIL8, GL_DEPTH24_STENCIL8);
    c.Add(T::DEPTH32F_STENCIL8, GL_DEPTH32F_STENCIL8);
    c.Add(T::STENCIL_INDEX8, GL_STENCIL_INDEX8);

    return c;
}();

static_assert(
    kGlTextureInternalFormatToGlValue.to_gl_value.Size() == kGlTextureInternalFormatToGlValue.to_gl_value.Capacity());
}  // namespace klgl::detail

namespace klgl
{

inline constexpr auto ToGlValue(GlTextureInternalFormat v)
{
    return detail::kGlTextureInternalFormatToGlValue.to_gl_value.Get(v);
}

}  // namespace klgl
