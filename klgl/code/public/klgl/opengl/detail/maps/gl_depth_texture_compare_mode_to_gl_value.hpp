#pragma once

#include "ass/enum_map.hpp"
#include "klgl/macro/ensure_enum_size.hpp"
#include "klgl/opengl/enums.hpp"

namespace klgl::detail
{
inline constexpr auto kGlDepthTextureCompareModeToGlValue = []
{
    using T = GlDepthTextureCompareMode;
    ass::EnumMap<T, GLint> map;

    auto add = [&](auto key, GLint value)
    {
        assert(!map.Contains(key));
        map.GetOrAdd(key) = value;
    };

    KLGL_ENSURE_ENUM_SIZE(T, 2);

    add(T::RefToTexture, GL_COMPARE_REF_TO_TEXTURE);
    add(T::None, GL_NONE);

    return map;
}();
}  // namespace klgl::detail
namespace klgl
{

[[nodiscard]] constexpr GLint ToGlValue(GlDepthTextureCompareMode buffer_type) noexcept
{
    return detail::kGlDepthTextureCompareModeToGlValue.Get(buffer_type);
}

}  // namespace klgl
