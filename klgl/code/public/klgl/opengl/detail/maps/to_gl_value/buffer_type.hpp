#pragma once

#include "klgl/macro/ensure_enum_size.hpp"
#include "klgl/opengl/enums.hpp"
#include "opengl_value_converter.hpp"

namespace klgl::detail
{
inline constexpr auto kGlBufferTypeConverter = []
{
    using T = GlBufferType;
    OpenGlValueConverter<T, GLenum> c;

    KLGL_ENSURE_ENUM_SIZE(T, 14);
    c.Add(T::Array, GL_ARRAY_BUFFER);
    c.Add(T::AtomicCounter, GL_ATOMIC_COUNTER_BUFFER);
    c.Add(T::CopyRead, GL_COPY_READ_BUFFER);
    c.Add(T::CopyWrite, GL_COPY_WRITE_BUFFER);
    c.Add(T::DispatchIndirect, GL_DISPATCH_INDIRECT_BUFFER);
    c.Add(T::DrawIndirect, GL_DRAW_INDIRECT_BUFFER);
    c.Add(T::ElementArray, GL_ELEMENT_ARRAY_BUFFER);
    c.Add(T::PixelPack, GL_PIXEL_PACK_BUFFER);
    c.Add(T::PixelUnpack, GL_PIXEL_UNPACK_BUFFER);
    c.Add(T::Query, GL_QUERY_BUFFER);
    c.Add(T::ShaderStorage, GL_SHADER_STORAGE_BUFFER);
    c.Add(T::Texture, GL_TEXTURE_BUFFER);
    c.Add(T::TransformFeedback, GL_TRANSFORM_FEEDBACK_BUFFER);
    c.Add(T::Uniform, GL_UNIFORM_BUFFER);

    return c;
}();
}  // namespace klgl::detail
namespace klgl
{

[[nodiscard]] constexpr auto ToGlValue(GlBufferType buffer_type) noexcept
{
    return detail::kGlBufferTypeConverter.to_gl_value.Get(buffer_type);
}

}  // namespace klgl
