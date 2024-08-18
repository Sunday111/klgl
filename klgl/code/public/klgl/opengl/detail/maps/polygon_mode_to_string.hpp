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
inline constexpr auto kGlPolygonModeToString = []
{
    ass::EnumMap<GlPolygonMode, std::string_view> map;

    auto add = [&](auto key, auto value)
    {
        assert(!map.Contains(key));
        map.GetOrAdd(key) = value;
    };

    KLGL_ENSURE_ENUM_SIZE(GlPolygonMode, 3);
    KLGL_STRING_MAP_ADD(GlPolygonMode::Point);
    KLGL_STRING_MAP_ADD(GlPolygonMode::Line);
    KLGL_STRING_MAP_ADD(GlPolygonMode::Fill);

    return map;
}();
}  // namespace klgl::detail

#undef KLGL_STRING_MAP_ADD
