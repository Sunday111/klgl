#pragma once

#include "klgl/opengl/gl_api.hpp"

namespace klgl::detail
{
template <typename T>
struct GlComponentTraits;

template <>
struct GlComponentTraits<float>
{
    static constexpr auto ComponentType = GlVertexAttribComponentType::Float;
};

template <>
struct GlComponentTraits<double>
{
    static constexpr auto ComponentType = GlVertexAttribComponentType::Double;
};

template <>
struct GlComponentTraits<int32_t>
{
    static constexpr auto ComponentType = GlVertexAttribComponentType::Int;
};

template <std::signed_integral T>
    requires(sizeof(T) > 1 && sizeof(T) <= 4)
struct GlComponentTraits<T>
{
    static constexpr auto ComponentType = GlVertexAttribComponentType::Int;
};

template <std::unsigned_integral T>
    requires(sizeof(T) > 1 && sizeof(T) <= 4)
struct GlComponentTraits<T>
{
    static constexpr auto ComponentType = GlVertexAttribComponentType::UnsignedInt;
};

template <>
struct GlComponentTraits<int8_t>
{
    static constexpr auto ComponentType = GlVertexAttribComponentType::Byte;
};

template <>
struct GlComponentTraits<uint8_t>
{
    static constexpr auto ComponentType = GlVertexAttribComponentType::UnsignedByte;
};
}  // namespace klgl::detail

namespace klgl
{

template <typename T>
struct TightlyPackedAttributeBuffer
{
    static void TightlyPackedAttributePointerAsFloat(uint32_t location, bool normalize)
        requires(std::same_as<T, float>)
    {
        klgl::OpenGl::VertexAttribPointer(
            location,
            1,
            detail::GlComponentTraits<float>::ComponentType,
            normalize,
            sizeof(float),
            nullptr);
    }

    static void TightlyPackedAttributePointerAsFloat(uint32_t location, bool normalize)
        requires(std::integral<T>)
    {
        klgl::OpenGl::VertexAttribPointer(
            location,
            1,
            detail::GlComponentTraits<T>::ComponentType,
            normalize,
            sizeof(T),
            nullptr);
    }

    static void TightlyPackedAttributePointerAsInt(uint32_t location)
        requires(std::integral<T>)
    {
        klgl::OpenGl::VertexAttribIPointer(
            location,
            1,
            detail::GlComponentTraits<T>::ComponentType,
            sizeof(T),
            nullptr);
    }

    // Source is the vector of floats
    static void TightlyPackedAttributePointerAsFloat(uint32_t location, bool normalize)
        requires(edt::IsMatrix<T> && T::IsVector() && std::same_as<typename T::Component, float>)
    {
        klgl::OpenGl::VertexAttribPointer(
            location,
            T::Size(),
            detail::GlComponentTraits<typename T::Component>::ComponentType,
            normalize,
            sizeof(T),
            nullptr);
    }

    // Source is the vector of integers converted to floats
    static void TightlyPackedAttributePointerAsFloat(uint32_t location, bool normalize)
        requires(edt::IsMatrix<T> && T::IsVector() && std::integral<typename T::Component>)
    {
        klgl::OpenGl::VertexAttribPointer(
            location,
            T::Size(),
            detail::GlComponentTraits<typename T::Component>::ComponentType,
            normalize,
            sizeof(T),
            nullptr);
    }

    // Source is the vector of integers that remain to be integers in the shader
    static void TightlyPackedAttributePointerAsInt(uint32_t location)
        requires(edt::IsMatrix<T> && T::IsVector() && std::integral<typename T::Component>)
    {
        klgl::OpenGl::VertexAttribIPointer(
            location,
            T::Size(),
            detail::GlComponentTraits<typename T::Component>::ComponentType,
            sizeof(T),
            nullptr);
    }

    // Source is a matrix of integers or floats that will be converted to matrix of floats
    static void TightlyPackedAttributePointerAsFloat(uint32_t location, bool normalize)
        requires(
            edt::IsMatrix<T> && !T::IsVector() &&
            (std::same_as<typename T::Component, float> || std::is_integral_v<typename T::Component>))
    {
        using Component = typename T::Component;
        static constexpr size_t rows = T::NumRows();
        for (GLuint i = 0; i != T::NumRows(); ++i)
        {
            klgl::OpenGl::VertexAttribPointer(
                location + i,
                rows,
                detail::GlComponentTraits<Component>::ComponentType,
                normalize,
                sizeof(T),
                reinterpret_cast<const void*>(sizeof(Component) * i * rows));  // NOLINT
        }
    }

    // Source is the matrix of integers
    static void TightlyPackedAttributePointerAsInt(uint32_t location)
        requires(edt::IsMatrix<T> && !T::IsVector() && std::is_integral_v<typename T::Component>)
    {
        using Component = typename T::Component;
        static constexpr size_t rows = T::NumRows();
        for (GLuint i = 0; i != T::NumRows(); ++i)
        {
            klgl::OpenGl::VertexAttribIPointer(
                location + i,
                rows,
                detail::GlComponentTraits<Component>::ComponentType,
                sizeof(T),
                reinterpret_cast<const void*>(sizeof(Component) * i * rows));  // NOLINT
        }
    }

    static constexpr size_t kLocationsCount = []() -> GLuint
    {
        if constexpr (edt::IsMatrix<T>)
        {
            if constexpr (!T::IsVector())
            {
                return T::NumRows();
            }
        }

        return 1;
    }();

    static void AttributeDivisor(GLuint location, GLint divisor)
    {
        for (GLuint i = 0; i != kLocationsCount; ++i)
        {
            glVertexAttribDivisor(location + i, divisor);
        }
    }

    static void EnableVertexAttribArray(GLuint location)
    {
        for (GLuint i = 0; i != kLocationsCount; ++i)
        {
            klgl::OpenGl::EnableVertexAttribArray(location + i);
        }
    }

    static void EnableVertexArrayAttrib(GlVertexArrayId vao, GLuint location)
    {
        for (GLuint i = 0; i != kLocationsCount; ++i)
        {
            glEnableVertexArrayAttrib(vao.GetValue(), location + i);
        }
    }
};

// Provides a function with a single interface
template <typename T, bool normalize, bool convert_to_float = true>
struct TightlyPackedAttributeBufferStatic
{
    static void AttributePointer(uint32_t location)
    {
        if constexpr (convert_to_float)
        {
            TightlyPackedAttributeBuffer<T>::TightlyPackedAttributePointerAsFloat(location, normalize);
        }
        else
        {
            TightlyPackedAttributeBuffer<T>::TightlyPackedAttributePointerAsInt(location);
        }
    }

    static void AttributeDivisor(GLuint location, GLint divisor)
    {
        TightlyPackedAttributeBuffer<T>::AttributeDivisor(location, divisor);
    }

    static void EnableVertexAttribArray(GLuint location)
    {
        TightlyPackedAttributeBuffer<T>::EnableVertexAttribArray(location);
    }

    static void EnableVertexArrayAttrib(GlVertexArrayId vao, GLuint location)
    {
        TightlyPackedAttributeBuffer<T>::EnableVertexArrayAttrib(vao, location);
    }
};

}  // namespace klgl
