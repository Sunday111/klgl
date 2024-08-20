#pragma once

#include "ass/enum_map.hpp"
#include "klgl/macro/ensure_enum_size.hpp"
#include "klgl/opengl/enums.hpp"

namespace klgl::detail
{
inline constexpr auto kGlTextureInternalFormatToGlValue = []
{
    using T = GlTextureInternalFormat;
    ass::EnumMap<T, GLint> map;

    auto add = [&](auto key, GLint value)
    {
        assert(!map.Contains(key));
        map.GetOrAdd(key) = value;
    };

    KLGL_ENSURE_ENUM_SIZE(T, 67);

    add(T::DEPTH_COMPONENT, GL_DEPTH_COMPONENT);
    add(T::DEPTH_STENCIL, GL_DEPTH_STENCIL);
    add(T::RED, GL_RED);
    add(T::RG, GL_RG);
    add(T::RGB, GL_RGB);
    add(T::RGBA, GL_RGBA);
    add(T::R8, GL_R8);
    add(T::R8_SNORM, GL_R8_SNORM);
    add(T::R16, GL_R16);
    add(T::R16_SNORM, GL_R16_SNORM);
    add(T::RG8, GL_RG8);
    add(T::RG8_SNORM, GL_RG8_SNORM);
    add(T::RG16, GL_RG16);
    add(T::RG16_SNORM, GL_RG16_SNORM);
    add(T::R3_G3_B2, GL_R3_G3_B2);
    add(T::RGB4, GL_RGB4);
    add(T::RGB5, GL_RGB5);
    add(T::RGB8, GL_RGB8);
    add(T::RGB8_SNORM, GL_RGB8_SNORM);
    add(T::RGB10, GL_RGB10);
    add(T::RGB12, GL_RGB12);
    add(T::RGB16_SNORM, GL_RGB16_SNORM);
    add(T::RGBA2, GL_RGBA2);
    add(T::RGBA4, GL_RGBA4);
    add(T::RGB5_A1, GL_RGB5_A1);
    add(T::RGBA8, GL_RGBA8);
    add(T::RGBA8_SNORM, GL_RGBA8_SNORM);
    add(T::RGB10_A2, GL_RGB10_A2);
    add(T::RGB10_A2UI, GL_RGB10_A2UI);
    add(T::RGBA12, GL_RGBA12);
    add(T::RGBA16, GL_RGBA16);
    add(T::SRGB8, GL_SRGB8);
    add(T::SRGB8_ALPHA8, GL_SRGB8_ALPHA8);
    add(T::R16F, GL_R16F);
    add(T::RG16F, GL_RG16F);
    add(T::RGB16F, GL_RGB16F);
    add(T::RGBA16F, GL_RGBA16F);
    add(T::R32F, GL_R32F);
    add(T::RG32F, GL_RG32F);
    add(T::RGB32F, GL_RGB32F);
    add(T::RGBA32F, GL_RGBA32F);
    add(T::R11F_G11F_B10F, GL_R11F_G11F_B10F);
    add(T::RGB9_E5, GL_RGB9_E5);
    add(T::R8I, GL_R8I);
    add(T::R8UI, GL_R8UI);
    add(T::R16I, GL_R16I);
    add(T::R16UI, GL_R16UI);
    add(T::R32I, GL_R32I);
    add(T::R32UI, GL_R32UI);
    add(T::RG8I, GL_RG8I);
    add(T::RG8UI, GL_RG8UI);
    add(T::RG16I, GL_RG16I);
    add(T::RG16UI, GL_RG16UI);
    add(T::RG32I, GL_RG32I);
    add(T::RG32UI, GL_RG32UI);
    add(T::RGB8I, GL_RGB8I);
    add(T::RGB8UI, GL_RGB8UI);
    add(T::RGB16I, GL_RGB16I);
    add(T::RGB16UI, GL_RGB16UI);
    add(T::RGB32I, GL_RGB32I);
    add(T::RGB32UI, GL_RGB32UI);
    add(T::RGBA8I, GL_RGBA8I);
    add(T::RGBA8UI, GL_RGBA8UI);
    add(T::RGBA16I, GL_RGBA16I);
    add(T::RGBA16UI, GL_RGBA16UI);
    add(T::RGBA32I, GL_RGBA32I);
    add(T::RGBA32UI, GL_RGBA32UI);

    return map;
}();
}  // namespace klgl::detail

namespace klgl
{

inline constexpr auto ToGlValue(GlTextureInternalFormat v)
{
    return detail::kGlTextureInternalFormatToGlValue.Get(v);
}

}  // namespace klgl
