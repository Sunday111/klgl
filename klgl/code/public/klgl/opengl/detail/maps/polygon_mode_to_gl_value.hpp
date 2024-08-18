#pragma once

#include <ass/enum_map.hpp>

#include "klgl/macro/ensure_enum_size.hpp"
#include "klgl/opengl/enums.hpp"

namespace klgl::detail
{
inline constexpr auto kGlPolygonModelToGlValue = []
{
    ass::EnumMap<GlPolygonMode, GLenum> map;

    auto add = [&](auto key, GLenum value)
    {
        assert(!map.Contains(key));
        map.GetOrAdd(key) = value;
    };

    KLGL_ENSURE_ENUM_SIZE(GlPolygonMode, 3);
    add(GlPolygonMode::Point, GL_POINT);
    add(GlPolygonMode::Line, GL_LINE);
    add(GlPolygonMode::Fill, GL_FILL);

    return map;
}();
}  // namespace klgl::detail
