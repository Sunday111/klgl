#pragma once

#include "CppReflection/GetStaticTypeInfo.hpp"
#include "klgl/opengl/identifiers.hpp"

namespace klgl
{

class SamplerUniform
{
public:
    uint8_t sampler_index;
    GlTextureId texture;
};

}  // namespace klgl

namespace cppreflection
{
template <>
struct TypeReflectionProvider<::klgl::SamplerUniform>
{
    [[nodiscard]] inline constexpr static auto ReflectType()
    {
        return StaticClassTypeInfo<::klgl::SamplerUniform>(
            "SamplerUniform",
            edt::GUID::Create("2FBEEB94-BBB3-491C-A299-AD1960641D3F"));
    }
};
}  // namespace cppreflection
