#pragma once

#include <ass/enum_map.hpp>

#include "klgl/macro/ensure_enum_size.hpp"
#include "klgl/opengl/enums.hpp"

namespace klgl::detail
{
inline constexpr auto kPixelBufferLayoutToNumChannels = []
{
    ass::EnumMap<GlPixelBufferLayout, uint8_t> map;

    auto add = [&](auto key, uint8_t value)
    {
        assert(!map.Contains(key));
        map.GetOrAdd(key) = value;
    };

    KLGL_ENSURE_ENUM_SIZE(GlPixelBufferLayout, 14);
    add(GlPixelBufferLayout::R, 1);
    add(GlPixelBufferLayout::R_Int, 1);
    add(GlPixelBufferLayout::Depth, 1);
    add(GlPixelBufferLayout::RG, 2);
    add(GlPixelBufferLayout::RG_Int, 2);
    add(GlPixelBufferLayout::RGB, 3);
    add(GlPixelBufferLayout::RGB_Int, 3);
    add(GlPixelBufferLayout::BGR, 3);
    add(GlPixelBufferLayout::BGR_Int, 3);
    add(GlPixelBufferLayout::RGBA, 4);
    add(GlPixelBufferLayout::RGBA_Int, 4);
    add(GlPixelBufferLayout::BGRA, 4);
    add(GlPixelBufferLayout::BGRA_Int, 4);
    add(GlPixelBufferLayout::DepthStencil, 2);

    return map;
}();
}  // namespace klgl::detail
