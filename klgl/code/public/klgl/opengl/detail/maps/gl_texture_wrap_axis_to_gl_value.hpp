#pragma once

#include "ass/enum_map.hpp"
#include "klgl/macro/ensure_enum_size.hpp"
#include "klgl/opengl/enums.hpp"

namespace klgl::detail
{
inline constexpr auto kGlTextureWrapAxisToGlValue = []
{
    using T = GlTextureWrapAxis;
    ass::EnumMap<T, GLint> map;

    auto add = [&](auto key, GLint value)
    {
        assert(!map.Contains(key));
        map.GetOrAdd(key) = value;
    };

    KLGL_ENSURE_ENUM_SIZE(T, 3);

    add(T::S, GL_TEXTURE_WRAP_S);
    add(T::R, GL_TEXTURE_WRAP_R);
    add(T::T, GL_TEXTURE_WRAP_T);

    return map;
}();
}  // namespace klgl::detail
namespace klgl
{

[[nodiscard]] constexpr GLint ToGlValue(GlTextureWrapAxis axis) noexcept
{
    return detail::kGlTextureWrapAxisToGlValue.Get(axis);
}

}  // namespace klgl
