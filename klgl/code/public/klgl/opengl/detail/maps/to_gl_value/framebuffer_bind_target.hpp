#pragma once

#include <ass/enum_map.hpp>

#include "klgl/opengl/detail/maps/to_gl_value/opengl_value_converter.hpp"
#include "klgl/opengl/enums.hpp"

namespace klgl::detail
{
inline constexpr auto kGlFramebufferBindTargetToGlValue = []
{
    using T = GlFramebufferBindTarget;
    OpenGlValueConverter<T, GLenum> c;

    c.Add(T::Read, GL_READ_FRAMEBUFFER);
    c.Add(T::Draw, GL_DRAW_FRAMEBUFFER);
    c.Add(T::DrawAndRead, GL_FRAMEBUFFER);

    return c;
}();

static_assert(
    kGlFramebufferBindTargetToGlValue.to_gl_value.Size() == kGlFramebufferBindTargetToGlValue.to_gl_value.Capacity());
}  // namespace klgl::detail

namespace klgl
{

[[nodiscard]] constexpr GLenum ToGlValue(GlFramebufferBindTarget value) noexcept
{
    return detail::kGlFramebufferBindTargetToGlValue.to_gl_value.Get(value);
}

}  // namespace klgl
