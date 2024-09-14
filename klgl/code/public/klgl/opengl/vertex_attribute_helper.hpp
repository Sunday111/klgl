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
struct VertexBufferHelper
{
    static void AttributePointerAsFloat(
        size_t location,
        bool normalize,
        size_t outer_object_size = sizeof(T),
        size_t member_offset = 0)
        requires(std::same_as<T, float>)
    {
        klgl::OpenGl::VertexAttribPointer(
            location,
            1,
            detail::GlComponentTraits<float>::ComponentType,
            normalize,
            outer_object_size,
            reinterpret_cast<const void*>(member_offset));  // NOLINT
    }

    static void AttributePointerAsFloat(
        size_t location,
        bool normalize,
        size_t outer_object_size = sizeof(T),
        size_t member_offset = 0)
        requires(std::integral<T>)
    {
        klgl::OpenGl::VertexAttribPointer(
            location,
            1,
            detail::GlComponentTraits<T>::ComponentType,
            normalize,
            outer_object_size,
            reinterpret_cast<const void*>(member_offset));  // NOLINT
    }

    static void AttributePointerAsInt(size_t location, size_t outer_object_size = sizeof(T), size_t member_offset = 0)
        requires(std::integral<T>)
    {
        klgl::OpenGl::VertexAttribIPointer(
            location,
            1,
            detail::GlComponentTraits<T>::ComponentType,
            outer_object_size,
            reinterpret_cast<const void*>(member_offset));  // NOLINT
    }

    // Source is the vector of floats
    static void AttributePointerAsFloat(
        size_t location,
        bool normalize,
        size_t outer_object_size = sizeof(T),
        size_t member_offset = 0)
        requires(edt::IsMatrix<T> && T::IsVector() && std::same_as<typename T::Component, float>)
    {
        klgl::OpenGl::VertexAttribPointer(
            location,
            T::Size(),
            detail::GlComponentTraits<typename T::Component>::ComponentType,
            normalize,
            outer_object_size,
            reinterpret_cast<const void*>(member_offset));  // NOLINT
    }

    // Source is the vector of integers converted to floats
    static void AttributePointerAsFloat(
        size_t location,
        bool normalize,
        size_t outer_object_size = sizeof(T),
        size_t member_offset = 0)
        requires(edt::IsMatrix<T> && T::IsVector() && std::integral<typename T::Component>)
    {
        klgl::OpenGl::VertexAttribPointer(
            location,
            T::Size(),
            detail::GlComponentTraits<typename T::Component>::ComponentType,
            normalize,
            outer_object_size,
            reinterpret_cast<const void*>(member_offset));  // NOLINT
    }

    // Source is the vector of integers that remain to be integers in the shader
    static void AttributePointerAsInt(size_t location, size_t outer_object_size = sizeof(T), size_t member_offset = 0)
        requires(edt::IsMatrix<T> && T::IsVector() && std::integral<typename T::Component>)
    {
        klgl::OpenGl::VertexAttribIPointer(
            location,
            T::Size(),
            detail::GlComponentTraits<typename T::Component>::ComponentType,
            outer_object_size,
            reinterpret_cast<const void*>(member_offset));  // NOLINT
    }

    // Source is a matrix of integers or floats that will be converted to matrix of floats
    static void AttributePointerAsFloat(
        size_t location,
        bool normalize,
        size_t outer_object_size = sizeof(T),
        size_t member_offset = 0)
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
                outer_object_size,
                reinterpret_cast<const void*>(member_offset + sizeof(Component) * i * rows));  // NOLINT
        }
    }

    // Source is the matrix of integers
    static void AttributePointerAsInt(size_t location, size_t outer_object_size = sizeof(T), size_t member_offset = 0)
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
                outer_object_size,
                reinterpret_cast<const void*>(member_offset + sizeof(Component) * i * rows));  // NOLINT
        }
    }

    static constexpr size_t kLocationsCount = []() -> size_t
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

    static void AttributeDivisor(size_t location, GLint divisor)
    {
        for (GLuint i = 0; i != kLocationsCount; ++i)
        {
            glVertexAttribDivisor(static_cast<GLuint>(location + i), divisor);
        }
    }

    static void EnableVertexAttribArray(size_t location)
    {
        for (GLuint i = 0; i != kLocationsCount; ++i)
        {
            klgl::OpenGl::EnableVertexAttribArray(location + i);
        }
    }

    static void EnableVertexArrayAttrib(GlVertexArrayId vao, size_t location)
    {
        for (GLuint i = 0; i != kLocationsCount; ++i)
        {
            glEnableVertexArrayAttrib(vao.GetValue(), static_cast<GLuint>(location + i));
        }
    }
};

// Provides a function with a single interface
template <typename T, bool normalize = false, bool convert_to_float = true>
struct VertexBufferHelperStatic
{
    static void AttributePointer(size_t location, size_t outer_object_size = sizeof(T), size_t member_offset = 0)
    {
        if constexpr (convert_to_float)
        {
            VertexBufferHelper<T>::AttributePointerAsFloat(location, normalize, outer_object_size, member_offset);
        }
        else
        {
            VertexBufferHelper<T>::AttributePointerAsInt(location, outer_object_size, member_offset);
        }
    }

    static void AttributeDivisor(size_t location, GLint divisor)
    {
        VertexBufferHelper<T>::AttributeDivisor(location, divisor);
    }

    static void EnableVertexAttribArray(size_t location) { VertexBufferHelper<T>::EnableVertexAttribArray(location); }

    static void EnableVertexArrayAttrib(GlVertexArrayId vao, size_t location)
    {
        VertexBufferHelper<T>::EnableVertexArrayAttrib(vao, location);
    }
};

}  // namespace klgl
