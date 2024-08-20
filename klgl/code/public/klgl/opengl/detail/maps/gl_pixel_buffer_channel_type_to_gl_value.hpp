#pragma once

#include <ass/enum_map.hpp>

#include "klgl/macro/ensure_enum_size.hpp"
#include "klgl/opengl/enums.hpp"

namespace klgl::detail
{

inline constexpr auto kGlPixelBufferLayoutToGlValue = []
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
}  // namespace klgl::detail

namespace klgl
{
[[nodiscard]] inline constexpr GLint ToGlValue(GlPixelBufferLayout v)
{
    return detail::kGlPixelBufferLayoutToGlValue.Get(v);
}
}  // namespace klgl
