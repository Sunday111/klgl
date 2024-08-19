#pragma once

#include <ass/enum_map.hpp>

#include "klgl/macro/ensure_enum_size.hpp"
#include "klgl/opengl/enums.hpp"

namespace klgl::detail
{
inline constexpr auto kGlIndexBufferElementTypeToGlValue = []
{
    ass::EnumMap<GlIndexBufferElementType, GLenum> map;

    auto add = [&](auto key, GLenum value)
    {
        assert(!map.Contains(key));
        map.GetOrAdd(key) = value;
    };

    KLGL_ENSURE_ENUM_SIZE(GlIndexBufferElementType, 3);
    add(GlIndexBufferElementType::UnsignedByte, GL_UNSIGNED_BYTE);
    add(GlIndexBufferElementType::UnsignedShort, GL_UNSIGNED_SHORT);
    add(GlIndexBufferElementType::UnsignedInt, GL_UNSIGNED_INT);

    return map;
}();
}  // namespace klgl::detail
namespace klgl
{

[[nodiscard]] constexpr GLenum ToGlValue(GlIndexBufferElementType buffer_type) noexcept
{
    return detail::kGlIndexBufferElementTypeToGlValue.Get(buffer_type);
}

}  // namespace klgl
