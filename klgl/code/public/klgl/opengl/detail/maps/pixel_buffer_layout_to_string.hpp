#pragma once

#include "ass/enum_map.hpp"
#include "klgl/macro/ensure_enum_size.hpp"
#include "klgl/opengl/enums.hpp"

#ifndef KLGL_STRING_MAP_ADD
#define KLGL_STRING_MAP_ADD(Token) add(Token, #Token)
#else
#error
#endif

namespace klgl::detail
{
inline constexpr auto kGlPixelBufferLayoutToString = []
{
    ass::EnumMap<GlPixelBufferLayout, std::string_view> map;

    auto add = [&](auto key, auto value)
    {
        assert(!map.Contains(key));
        map.GetOrAdd(key) = value;
    };

    KLGL_ENSURE_ENUM_SIZE(GlPixelBufferLayout, 14);
    KLGL_STRING_MAP_ADD(GlPixelBufferLayout::R);
    KLGL_STRING_MAP_ADD(GlPixelBufferLayout::R_Int);
    KLGL_STRING_MAP_ADD(GlPixelBufferLayout::Depth);
    KLGL_STRING_MAP_ADD(GlPixelBufferLayout::RG);
    KLGL_STRING_MAP_ADD(GlPixelBufferLayout::RG_Int);
    KLGL_STRING_MAP_ADD(GlPixelBufferLayout::RGB);
    KLGL_STRING_MAP_ADD(GlPixelBufferLayout::RGB_Int);
    KLGL_STRING_MAP_ADD(GlPixelBufferLayout::BGR);
    KLGL_STRING_MAP_ADD(GlPixelBufferLayout::BGR_Int);
    KLGL_STRING_MAP_ADD(GlPixelBufferLayout::RGBA);
    KLGL_STRING_MAP_ADD(GlPixelBufferLayout::RGBA_Int);
    KLGL_STRING_MAP_ADD(GlPixelBufferLayout::BGRA);
    KLGL_STRING_MAP_ADD(GlPixelBufferLayout::BGRA_Int);
    KLGL_STRING_MAP_ADD(GlPixelBufferLayout::DepthStencil);

    return map;
}();
}  // namespace klgl::detail

#undef KLGL_STRING_MAP_ADD
