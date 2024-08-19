#pragma once

#include "ass/enum_map.hpp"
#include "klgl/macro/ensure_enum_size.hpp"
#include "klgl/opengl/enums.hpp"

namespace klgl::detail
{
inline constexpr auto kGlPrimitiveTypeToGlValue = []
{
    ass::EnumMap<GlPrimitiveType, GLenum> map;

    auto add = [&](auto key, GLenum value)
    {
        assert(!map.Contains(key));
        map.GetOrAdd(key) = value;
    };

    KLGL_ENSURE_ENUM_SIZE(GlPrimitiveType, 12);
    add(GlPrimitiveType::Points, GL_POINTS);
    add(GlPrimitiveType::LineStrip, GL_LINE_STRIP);
    add(GlPrimitiveType::LineLoop, GL_LINE_LOOP);
    add(GlPrimitiveType::Lines, GL_LINES);
    add(GlPrimitiveType::LineStripAdjacency, GL_LINE_STRIP_ADJACENCY);
    add(GlPrimitiveType::LinesAdjacency, GL_LINES_ADJACENCY);
    add(GlPrimitiveType::TriangleStrip, GL_TRIANGLE_STRIP);
    add(GlPrimitiveType::TriangleFan, GL_TRIANGLE_FAN);
    add(GlPrimitiveType::Triangles, GL_TRIANGLES);
    add(GlPrimitiveType::TriangleStripAdjacency, GL_TRIANGLE_STRIP_ADJACENCY);
    add(GlPrimitiveType::TrianglesAdjacency, GL_TRIANGLES_ADJACENCY);
    add(GlPrimitiveType::Patches, GL_PATCHES);

    return map;
}();
}  // namespace klgl::detail
namespace klgl
{

[[nodiscard]] constexpr GLenum ToGlValue(GlPrimitiveType buffer_type) noexcept
{
    return detail::kGlPrimitiveTypeToGlValue.Get(buffer_type);
}

}  // namespace klgl
