#pragma once

#include <glad/glad.h>

#include <bit>
#include <cstddef>

namespace klgl
{
using GlConstexprIntHasher = decltype([](const GLint v) { return std::bit_cast<size_t>(static_cast<int64_t>(v)); });
using GlConstexprEnumHasher = decltype([](const GLenum v) -> size_t { return v; });
}  // namespace klgl
