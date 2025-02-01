#pragma once

#include <ass/enum_map.hpp>

#include "klgl/opengl/detail/maps/to_gl_value/opengl_value_converter.hpp"
#include "klgl/opengl/enums.hpp"

namespace klgl::detail
{
inline constexpr auto kGlFramebufferAttachmentToGlValue = []
{
    using T = GlFramebufferAttachment;
    OpenGlValueConverter<T, GLenum> c;

    c.Add(T::Color0, GL_COLOR_ATTACHMENT0);
    c.Add(T::Color1, GL_COLOR_ATTACHMENT1);
    c.Add(T::Color2, GL_COLOR_ATTACHMENT2);
    c.Add(T::Color3, GL_COLOR_ATTACHMENT3);
    c.Add(T::Color4, GL_COLOR_ATTACHMENT4);
    c.Add(T::Color5, GL_COLOR_ATTACHMENT5);
    c.Add(T::Color6, GL_COLOR_ATTACHMENT6);
    c.Add(T::Color7, GL_COLOR_ATTACHMENT7);
    c.Add(T::Depth, GL_DEPTH_ATTACHMENT);
    c.Add(T::Stencil, GL_STENCIL_ATTACHMENT);
    c.Add(T::DepthStencil, GL_DEPTH_STENCIL_ATTACHMENT);

    return c;
}();

static_assert(
    kGlFramebufferAttachmentToGlValue.to_gl_value.Size() == kGlFramebufferAttachmentToGlValue.to_gl_value.Capacity());
}  // namespace klgl::detail

namespace klgl
{

[[nodiscard]] constexpr GLenum ToGlValue(GlFramebufferAttachment value) noexcept
{
    return detail::kGlFramebufferAttachmentToGlValue.to_gl_value.Get(value);
}

}  // namespace klgl
