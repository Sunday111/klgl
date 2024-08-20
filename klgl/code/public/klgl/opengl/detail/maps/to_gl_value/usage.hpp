#pragma once

#include <ass/enum_map.hpp>

#include "klgl/macro/ensure_enum_size.hpp"
#include "klgl/opengl/detail/maps/to_gl_value/opengl_value_converter.hpp"
#include "klgl/opengl/enums.hpp"

namespace klgl::detail
{
inline constexpr auto kGlUsageToGlValue = []
{
    using T = GlUsage;
    OpenGlValueConverter<T, GLenum> c;

    KLGL_ENSURE_ENUM_SIZE(T, 9);
    c.Add(T::StreamDraw, GL_STREAM_DRAW);
    c.Add(T::StreamRead, GL_STREAM_READ);
    c.Add(T::StreamCopy, GL_STREAM_COPY);
    c.Add(T::StaticDraw, GL_STATIC_DRAW);
    c.Add(T::StaticRead, GL_STATIC_READ);
    c.Add(T::StaticCopy, GL_STATIC_COPY);
    c.Add(T::DynamicDraw, GL_DYNAMIC_DRAW);
    c.Add(T::DynamicRead, GL_DYNAMIC_READ);
    c.Add(T::DynamicCopy, GL_DYNAMIC_COPY);

    return c;
}();
}  // namespace klgl::detail

namespace klgl
{

[[nodiscard]] constexpr GLenum ToGlValue(GlUsage usage) noexcept
{
    return detail::kGlUsageToGlValue.to_gl_value.Get(usage);
}

}  // namespace klgl
