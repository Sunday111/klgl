#pragma once

#include "ass/enum_map.hpp"
#include "klgl/macro/ensure_enum_size.hpp"
#include "klgl/opengl/enums.hpp"

namespace klgl::detail
{
inline constexpr auto kGlDepthTextureCompareFunctionToGlValue = []
{
    using T = GlDepthTextureCompareFunction;
    ass::EnumMap<T, GLint> map;

    auto add = [&](auto key, GLint value)
    {
        assert(!map.Contains(key));
        map.GetOrAdd(key) = value;
    };

    KLGL_ENSURE_ENUM_SIZE(T, 8);

    add(T::LessOrEqual, GL_LEQUAL);
    add(T::GreaterOrEqual, GL_GEQUAL);
    add(T::Less, GL_LESS);
    add(T::Greater, GL_GREATER);
    add(T::Equal, GL_EQUAL);
    add(T::NotEqual, GL_NOTEQUAL);
    add(T::Always, GL_ALWAYS);
    add(T::Never, GL_NEVER);

    return map;
}();
}  // namespace klgl::detail
namespace klgl
{

[[nodiscard]] constexpr GLint ToGlValue(GlDepthTextureCompareFunction buffer_type) noexcept
{
    return detail::kGlDepthTextureCompareFunctionToGlValue.Get(buffer_type);
}

}  // namespace klgl
