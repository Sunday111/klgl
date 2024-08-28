#pragma once

#include <CppReflection/GetTypeInfo.hpp>

#include "klgl/reflection/matrix_reflect.hpp"  // IWYU pragma: keep
#include "type_id_widget_minimal.hpp"

namespace klgl
{
template <typename T>
inline bool SimpleTypeWidget(std::string_view name, T& value)
{
    bool value_changed = false;
    klgl::SimpleTypeWidget(cppreflection::GetTypeInfo<T>()->GetGuid(), name, &value, value_changed);
    return value_changed;
}
}  // namespace klgl
