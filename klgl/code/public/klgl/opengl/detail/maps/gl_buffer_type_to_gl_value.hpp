#pragma once

#include <ass/enum_map.hpp>

#include "klgl/macro/ensure_enum_size.hpp"
#include "klgl/opengl/enums.hpp"

namespace klgl::detail
{
inline constexpr auto kGlBufferTypeToGlValue = []
{
    ass::EnumMap<GlBufferType, GLenum> map;

    auto add = [&](auto key, GLenum value)
    {
        assert(!map.Contains(key));
        map.GetOrAdd(key) = value;
    };

    KLGL_ENSURE_ENUM_SIZE(GlBufferType, 14);
    add(GlBufferType::Array, GL_ARRAY_BUFFER);
    add(GlBufferType::AtomicCounter, GL_ATOMIC_COUNTER_BUFFER);
    add(GlBufferType::CopyRead, GL_COPY_READ_BUFFER);
    add(GlBufferType::CopyWrite, GL_COPY_WRITE_BUFFER);
    add(GlBufferType::DispatchIndirect, GL_DISPATCH_INDIRECT_BUFFER);
    add(GlBufferType::DrawIndirect, GL_DRAW_INDIRECT_BUFFER);
    add(GlBufferType::ElementArray, GL_ELEMENT_ARRAY_BUFFER);
    add(GlBufferType::PixelPack, GL_PIXEL_PACK_BUFFER);
    add(GlBufferType::PixelUnpack, GL_PIXEL_UNPACK_BUFFER);
    add(GlBufferType::Query, GL_QUERY_BUFFER);
    add(GlBufferType::ShaderStorage, GL_SHADER_STORAGE_BUFFER);
    add(GlBufferType::Texture, GL_TEXTURE_BUFFER);
    add(GlBufferType::TransformFeedback, GL_TRANSFORM_FEEDBACK_BUFFER);
    add(GlBufferType::Uniform, GL_UNIFORM_BUFFER);

    return map;
}();
}  // namespace klgl::detail
namespace klgl
{

[[nodiscard]] constexpr GLenum ToGlValue(GlBufferType buffer_type) noexcept
{
    return detail::kGlBufferTypeToGlValue.Get(buffer_type);
}

}  // namespace klgl
