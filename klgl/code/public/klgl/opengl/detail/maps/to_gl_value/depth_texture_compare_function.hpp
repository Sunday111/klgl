#pragma once

#include "ass/enum_map.hpp"
#include "klgl/macro/ensure_enum_size.hpp"
#include "klgl/opengl/detail/maps/to_gl_value/opengl_value_converter.hpp"
#include "klgl/opengl/enums.hpp"

namespace klgl::detail
{
inline constexpr auto kGlDepthTextureCompareFunctionToGlValue = []
{
    using T = GlDepthTextureCompareFunction;
    OpenGlValueConverter<T, GLint> c;

    KLGL_ENSURE_ENUM_SIZE(T, 8);
    c.Add(T::LessOrEqual, GL_LEQUAL);
    c.Add(T::GreaterOrEqual, GL_GEQUAL);
    c.Add(T::Less, GL_LESS);
    c.Add(T::Greater, GL_GREATER);
    c.Add(T::Equal, GL_EQUAL);
    c.Add(T::NotEqual, GL_NOTEQUAL);
    c.Add(T::Always, GL_ALWAYS);
    c.Add(T::Never, GL_NEVER);

    return c;
}();
}  // namespace klgl::detail
namespace klgl
{

[[nodiscard]] constexpr auto ToGlValue(GlDepthTextureCompareFunction buffer_type) noexcept
{
    return detail::kGlDepthTextureCompareFunctionToGlValue.to_gl_value.Get(buffer_type);
}

}  // namespace klgl
