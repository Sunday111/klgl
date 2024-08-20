#pragma once

#include "klgl/macro/ensure_enum_size.hpp"
#include "klgl/opengl/detail/maps/to_gl_value/opengl_value_converter.hpp"
#include "klgl/opengl/enums.hpp"

namespace klgl::detail
{
inline constexpr auto kGlPrimitiveTypeToGlValue = []
{
    using T = GlPrimitiveType;
    OpenGlValueConverter<T, GLenum> c;

    KLGL_ENSURE_ENUM_SIZE(GlPrimitiveType, 12);
    c.Add(T::Points, GL_POINTS);
    c.Add(T::LineStrip, GL_LINE_STRIP);
    c.Add(T::LineLoop, GL_LINE_LOOP);
    c.Add(T::Lines, GL_LINES);
    c.Add(T::LineStripAdjacency, GL_LINE_STRIP_ADJACENCY);
    c.Add(T::LinesAdjacency, GL_LINES_ADJACENCY);
    c.Add(T::TriangleStrip, GL_TRIANGLE_STRIP);
    c.Add(T::TriangleFan, GL_TRIANGLE_FAN);
    c.Add(T::Triangles, GL_TRIANGLES);
    c.Add(T::TriangleStripAdjacency, GL_TRIANGLE_STRIP_ADJACENCY);
    c.Add(T::TrianglesAdjacency, GL_TRIANGLES_ADJACENCY);
    c.Add(T::Patches, GL_PATCHES);

    return c;
}();
}  // namespace klgl::detail
namespace klgl
{

[[nodiscard]] constexpr GLenum ToGlValue(GlPrimitiveType buffer_type) noexcept
{
    return detail::kGlPrimitiveTypeToGlValue.to_gl_value.Get(buffer_type);
}

}  // namespace klgl
