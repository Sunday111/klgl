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
    static constexpr size_t Stride = sizeof(float);
    static constexpr auto AttribComponentType = GlVertexAttribComponentType::Float;

    static void VertexAttributePointer(GLuint location, bool normalize)
    {
        klgl::OpenGl::VertexAttribPointer(location, Size, AttribComponentType, normalize, Stride, nullptr);
    }

    static void VertexAttributeDivisor(GLuint location, GLint divisor) { glVertexAttribDivisor(location, divisor); }
};

template <>
struct TypeToGlType<uint8_t>
{
    static constexpr size_t Size = 1;
    static constexpr GLenum Type = GL_UNSIGNED_BYTE;
    static constexpr size_t Stride = sizeof(uint8_t);
    static constexpr auto AttribComponentType = GlVertexAttribComponentType::UnsignedByte;

    static void VertexAttributePointer(GLuint location, bool normalize)
    {
        klgl::OpenGl::VertexAttribPointer(location, Size, AttribComponentType, normalize, Stride, nullptr);
    }

    static void VertexAttributeDivisor(GLuint location, GLint divisor) { glVertexAttribDivisor(location, divisor); }
};

template <>
struct TypeToGlType<uint32_t>
{
    static constexpr size_t Size = 1;
    static constexpr GLenum Type = GL_UNSIGNED_INT;
    static constexpr size_t Stride = sizeof(uint32_t);
    static constexpr auto AttribComponentType = GlVertexAttribComponentType::UnsignedInt;

    static void VertexAttributePointer(GLuint location, bool normalize)
    {
        klgl::OpenGl::VertexAttribPointer(location, Size, AttribComponentType, normalize, Stride, nullptr);
    }

    static void VertexAttributeDivisor(GLuint location, GLint divisor) { glVertexAttribDivisor(location, divisor); }
};

template <typename T, size_t N>
struct TypeToGlType<edt::Matrix<T, N, 1>>
{
    static constexpr size_t Size = N;
    static constexpr GLenum Type = TypeToGlType<T>::Type;
    static constexpr size_t Stride = N * sizeof(T);
    static constexpr auto AttribComponentType = TypeToGlType<T>::AttribComponentType;

    static void VertexAttributePointer(GLuint location, bool normalize)
    {
        klgl::OpenGl::VertexAttribPointer(location, Size, AttribComponentType, normalize, Stride, nullptr);
    }

    static void VertexAttributeDivisor(GLuint location, GLint divisor) { glVertexAttribDivisor(location, divisor); }
};

template <typename T, size_t N, size_t R>
    requires(R > 1 && R == N)
struct TypeToGlType<edt::Matrix<T, N, R>>
{
    static constexpr size_t Size = N;
    static constexpr GLenum Type = TypeToGlType<T>::Type;
    static constexpr size_t Stride = sizeof(T) * R * N;
    static constexpr auto AttribComponentType = TypeToGlType<T>::AttribComponentType;

    static void VertexAttributePointer(GLuint location, bool normalize)
    {
        for (GLuint i = 0; i != R; ++i)
        {
            klgl::OpenGl::VertexAttribPointer(
                location + i,
                R,
                AttribComponentType,
                normalize,
                Stride,
                reinterpret_cast<const void*>(sizeof(T) * i * R));  // NOLINT
        }
    }

    static void VertexAttributeDivisor(GLuint location, GLint divisor)
    {
        for (GLuint i = 0; i != R; ++i)
        {
            glVertexAttribDivisor(location + i, divisor);
        }
    }
};

}  // namespace klgl
