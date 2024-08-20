#pragma once

#include <ass/enum_map.hpp>

#include "klgl/macro/ensure_enum_size.hpp"
#include "klgl/opengl/detail/maps/to_gl_value/opengl_value_converter.hpp"
#include "klgl/opengl/enums.hpp"

namespace klgl::detail
{
inline constexpr auto kGlPixelBufferChannelTypeToGlValue = []
{
    using T = GlPixelBufferChannelType;
    OpenGlValueConverter<T, GLint> c;

    KLGL_ENSURE_ENUM_SIZE(T, 2);
    c.Add(T::UByte, GL_UNSIGNED_BYTE);
    c.Add(T::Float, GL_FLOAT);

    return c;
}();
}  // namespace klgl::detail

namespace klgl
{
[[nodiscard]] inline constexpr auto ToGlValue(GlPixelBufferChannelType v)
{
    return detail::kGlPixelBufferChannelTypeToGlValue.to_gl_value.Get(v);
}
}  // namespace klgl
