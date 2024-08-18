#pragma once

#include <ass/fixed_unordered_map.hpp>

#include "klgl/macro/ensure_enum_size.hpp"
#include "klgl/opengl/enums.hpp"
#include "klgl/opengl/hash.hpp"

namespace klgl
{
inline constexpr auto kGlValueToGlError = []
{
    constexpr size_t enum_size = magic_enum::enum_count<GlError>();
    ass::FixedUnorderedMap<enum_size, GLenum, GlError, GlConstexprEnumHasher> map;

    auto add = [&](GLenum key, GlError value)
    {
        assert(!map.Contains(key));
        map.Add(key, value);
    };

    KLGL_ENSURE_ENUM_SIZE(GlError, 8);
    add(GL_NO_ERROR, GlError::NoError);
    add(GL_INVALID_ENUM, GlError::InvalidEnum);
    add(GL_INVALID_VALUE, GlError::InvalidValue);
    add(GL_INVALID_OPERATION, GlError::InvalidOperation);
    add(GL_STACK_OVERFLOW, GlError::StackOverflow);
    add(GL_STACK_UNDERFLOW, GlError::StackUnderflow);
    add(GL_OUT_OF_MEMORY, GlError::OutOfMemory);

    return map;
}();
}  // namespace klgl
