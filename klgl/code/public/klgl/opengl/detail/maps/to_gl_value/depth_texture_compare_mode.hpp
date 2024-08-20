#pragma once

#include "klgl/macro/ensure_enum_size.hpp"
#include "klgl/opengl/detail/maps/to_gl_value/opengl_value_converter.hpp"
#include "klgl/opengl/enums.hpp"

namespace klgl::detail
{
inline constexpr auto kGlDepthTextureCompareModeToGlValue = []
{
    using T = GlDepthTextureCompareMode;
    OpenGlValueConverter<T, GLint> c;

    KLGL_ENSURE_ENUM_SIZE(T, 2);
    c.Add(T::RefToTexture, GL_COMPARE_REF_TO_TEXTURE);
    c.Add(T::None, GL_NONE);

    return c;
}();
}  // namespace klgl::detail
namespace klgl
{

[[nodiscard]] constexpr auto ToGlValue(GlDepthTextureCompareMode buffer_type) noexcept
{
    return detail::kGlDepthTextureCompareModeToGlValue.to_gl_value.Get(buffer_type);
}

}  // namespace klgl
