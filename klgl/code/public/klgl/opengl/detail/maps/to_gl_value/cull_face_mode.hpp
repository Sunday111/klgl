#pragma once

#include "klgl/macro/ensure_enum_size.hpp"
#include "klgl/opengl/detail/maps/to_gl_value/opengl_value_converter.hpp"
#include "klgl/opengl/enums.hpp"

namespace klgl::detail
{
inline constexpr auto kGlCullFaceModeToGlValue = []
{
    using T = GlCullFaceMode;
    OpenGlValueConverter<T, GLenum> c;

    KLGL_ENSURE_ENUM_SIZE(T, 3);
    c.Add(T::Front, GL_FRONT);
    c.Add(T::Back, GL_BACK);
    c.Add(T::Both, GL_FRONT_AND_BACK);

    return c;
}();
}  // namespace klgl::detail
namespace klgl
{

[[nodiscard]] constexpr GLenum ToGlValue(GlCullFaceMode shader_type) noexcept
{
    return detail::kGlCullFaceModeToGlValue.to_gl_value.Get(shader_type);
}

}  // namespace klgl
