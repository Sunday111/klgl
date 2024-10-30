#pragma once

#include <CppReflection/GetTypeInfo.hpp>

#include "klgl/reflection/matrix_reflect.hpp"  // IWYU pragma: keep
#include "type_id_widget_minimal.hpp"

namespace klgl
{
template <typename T>
inline bool SimpleTypeWidget(std::string_view name, T& value)
{
    return klgl::SimpleTypeWidget(cppreflection::GetTypeInfo<std::remove_const_t<T>>()->GetGuid(), name, &value);
}

}  // namespace klgl
