#pragma once

#include "EverydayTools/Math/Matrix.hpp"
#include "klgl/opengl/enums.hpp"

#ifdef KLGL_MAPPER
#pragma error "Error: macro already dfined"
#else
#define KLGL_MAPPER(Type, Value)                              \
    template <>                                               \
    struct TypeVertAttribTypeEnum<Type>                       \
    {                                                         \
        static constexpr GlVertexAttributeType value = Value; \
    }
#endif

namespace klgl::detail
{
template <typename T>
struct TypeVertAttribTypeEnum;

KLGL_MAPPER(float, GlVertexAttributeType::Float);
KLGL_MAPPER(edt::Vec2f, GlVertexAttributeType::FloatVec2);
KLGL_MAPPER(edt::Vec3f, GlVertexAttributeType::FloatVec3);
KLGL_MAPPER(edt::Vec4f, GlVertexAttributeType::FloatVec4);
KLGL_MAPPER(edt::Mat2f, GlVertexAttributeType::FloatMat2);
KLGL_MAPPER(edt::Mat3f, GlVertexAttributeType::FloatMat3);
KLGL_MAPPER(edt::Mat4f, GlVertexAttributeType::FloatMat4);

template <std::signed_integral T>
    requires(sizeof(T) <= 4)
struct TypeVertAttribTypeEnum<T>
{
    static constexpr auto value = GlVertexAttributeType::Int;
};

template <std::unsigned_integral T>
    requires(sizeof(T) <= 4)
struct TypeVertAttribTypeEnum<T>
{
    static constexpr auto value = GlVertexAttributeType::UnsigneInt;
};

}  // namespace klgl::detail

#undef KLGL_MAPPER
