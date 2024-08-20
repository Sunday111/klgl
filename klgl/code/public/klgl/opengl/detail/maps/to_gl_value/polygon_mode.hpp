#pragma once

#include <ass/enum_map.hpp>

#include "klgl/macro/ensure_enum_size.hpp"
#include "klgl/opengl/detail/maps/to_gl_value/opengl_value_converter.hpp"
#include "klgl/opengl/enums.hpp"

namespace klgl::detail
{
inline constexpr auto kGlPolygonModelToGlValue = []
{
    using T = GlPolygonMode;
    OpenGlValueConverter<T, GLenum> c;

    KLGL_ENSURE_ENUM_SIZE(T, 3);
    c.Add(T::Point, GL_POINT);
    c.Add(T::Line, GL_LINE);
    c.Add(T::Fill, GL_FILL);

    return c;
}();

}  // namespace klgl::detail

namespace klgl
{
[[nodiscard]] inline constexpr GLenum ToGlValue(GlPolygonMode mode) noexcept
{
    return detail::kGlPolygonModelToGlValue.to_gl_value.Get(mode);
}
}  // namespace klgl
