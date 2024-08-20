#pragma once

#include "ass/enum_map.hpp"
#include "klgl/macro/ensure_enum_size.hpp"
#include "klgl/opengl/enums.hpp"

namespace klgl::detail
{
inline constexpr auto kGlVertexAttribComponentTypeToGlValue = []
{
    using T = GlVertexAttribComponentType;
    ass::EnumMap<T, GLenum> map;

    auto add = [&](auto key, GLenum value)
    {
        assert(!map.Contains(key));
        map.GetOrAdd(key) = value;
    };

    KLGL_ENSURE_ENUM_SIZE(T, 13);

    add(T::Byte, GL_BYTE);
    add(T::UnsignedByte, GL_UNSIGNED_BYTE);
    add(T::Short, GL_SHORT);
    add(T::UnsignedShort, GL_UNSIGNED_SHORT);
    add(T::Int, GL_INT);
    add(T::UnsignedInt, GL_UNSIGNED_INT);
    add(T::HalfFloat, GL_HALF_FLOAT);
    add(T::Float, GL_FLOAT);
    add(T::Double, GL_DOUBLE);
    add(T::Fixed, GL_FIXED);
    add(T::Int_2_10_10_10_Rev, GL_INT_2_10_10_10_REV);
    add(T::UnsignedInt_2_10_10_10_Rev, GL_UNSIGNED_INT_2_10_10_10_REV);
    add(T::UnsignedInt_10F_11F_11F_Rev, GL_UNSIGNED_INT_10F_11F_11F_REV);

    return map;
}();
}  // namespace klgl::detail
namespace klgl
{

[[nodiscard]] constexpr GLenum ToGlValue(GlVertexAttribComponentType buffer_type) noexcept
{
    return detail::kGlVertexAttribComponentTypeToGlValue.Get(buffer_type);
}

}  // namespace klgl
