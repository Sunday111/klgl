#pragma once

#include "klgl/opengl/gl_api.hpp"

namespace klgl
{

template <typename T>
struct TypeToGlType;

template <>
struct TypeToGlType<float>
{
    static constexpr size_t Size = 1;
    static constexpr GLenum Type = GL_FLOAT;
    static constexpr auto AttribComponentType = GlVertexAttribComponentType::Float;
};

template <>
struct TypeToGlType<uint8_t>
{
    static constexpr size_t Size = 1;
    static constexpr GLenum Type = GL_UNSIGNED_BYTE;
    static constexpr auto AttribComponentType = GlVertexAttribComponentType::UnsignedByte;
};

template <typename T, int N>
struct TypeToGlType<edt::Matrix<T, N, 1>>
{
    static constexpr size_t Size = static_cast<size_t>(N);
    static constexpr GLenum Type = TypeToGlType<T>::Type;
    static constexpr auto AttribComponentType = TypeToGlType<T>::AttribComponentType;
};

}  // namespace klgl
