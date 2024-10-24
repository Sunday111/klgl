#include <fmt/format.h>
#include <imgui.h>

#include <limits>

#include "CppReflection/GetStaticTypeInfo.hpp"
#include "CppReflection/TypeRegistry.hpp"
#include "klgl/error_handling.hpp"
#include "klgl/reflection/matrix_reflect.hpp"  // IWYU pragma: keep
#include "klgl/ui/type_id_widget_minimal.hpp"

namespace klgl
{

using namespace edt::lazy_matrix_aliases;  // NOLINT

template <typename T>
static constexpr ImGuiDataType_ CastDataType() noexcept
{
    if constexpr (std::is_same_v<T, int8_t>)
    {
        return ImGuiDataType_S8;
    }
    if constexpr (std::is_same_v<T, uint8_t>)
    {
        return ImGuiDataType_U8;
    }
    if constexpr (std::is_same_v<T, int16_t>)
    {
        return ImGuiDataType_S16;
    }
    if constexpr (std::is_same_v<T, uint16_t>)
    {
        return ImGuiDataType_U16;
    }
    if constexpr (std::is_same_v<T, int32_t>)
    {
        return ImGuiDataType_S32;
    }
    if constexpr (std::is_same_v<T, uint32_t>)
    {
        return ImGuiDataType_U32;
    }
    if constexpr (std::is_same_v<T, int64_t>)
    {
        return ImGuiDataType_S64;
    }
    if constexpr (std::is_same_v<T, uint64_t>)
    {
        return ImGuiDataType_U64;
    }
    if constexpr (std::is_same_v<T, float>)
    {
        return ImGuiDataType_Float;
    }
    if constexpr (std::is_same_v<T, double>)
    {
        return ImGuiDataType_Double;
    }
}

template <typename T>
bool ScalarProperty(
    edt::GUID type_guid,
    std::string_view name,
    void* address,
    bool& value_changed,
    T min = std::numeric_limits<T>::lowest(),
    T max = std::numeric_limits<T>::max())
{
    constexpr auto type_info = cppreflection::GetStaticTypeInfo<T>();
    if (type_info.guid != type_guid) return false;
    auto value = reinterpret_cast<T*>(address);  // NOLINT
    const bool c = ImGui::DragScalar(name.data(), CastDataType<T>(), value, 1.0f, &min, &max);
    value_changed = c;
    return true;
}

template <typename T>
bool ScalarProperty(edt::GUID type_guid, std::string_view name, const void* address)
{
    constexpr T min = std::numeric_limits<T>::lowest();
    constexpr T max = std::numeric_limits<T>::max();
    constexpr auto type_info = cppreflection::GetStaticTypeInfo<T>();
    if (type_info.guid != type_guid) return false;
    auto value = *reinterpret_cast<const T*>(address);  // NOLINT
    ImGui::BeginDisabled(true);
    ImGui::DragScalar(name.data(), CastDataType<T>(), &value, 1.0f, &min, &max);
    ImGui::EndDisabled();
    return true;
}

template <typename T, size_t N>
bool VectorProperty(
    std::string_view title,
    edt::Matrix<T, N, 1>& value,
    T min = std::numeric_limits<T>::lowest(),
    T max = std::numeric_limits<T>::max()) noexcept
{
    return ImGui::DragScalarN(title.data(), CastDataType<T>(), value.data(), N, 0.01f, &min, &max, "%.3f");
}

template <typename T>
bool VectorProperty(edt::GUID type_guid, std::string_view name, void* address, bool& value_changed)
{
    constexpr auto type_info = cppreflection::GetStaticTypeInfo<T>();
    if (type_info.guid == type_guid)
    {
        T& member_ref = *reinterpret_cast<T*>(address);  // NOLINT
        value_changed |= VectorProperty(name, member_ref);
        return true;
    }

    return false;
}

template <typename T, size_t num_rows, size_t num_columns>
bool MatrixProperty(
    const std::string_view title,
    edt::Matrix<T, num_rows, num_columns>& value,
    T min = std::numeric_limits<T>::lowest(),
    T max = std::numeric_limits<T>::max()) noexcept
{
    bool changed = false;
    if (ImGui::TreeNode(title.data()))
    {
        for (size_t row_index = 0; row_index != num_rows; ++row_index)
        {
            edt::Matrix<T, num_columns, 1> row = value.GetRow(row_index).Transposed();
            ImGui::PushID(static_cast<int>(row_index));
            const bool row_changed = ImGui::DragScalarN(
                "",
                CastDataType<T>(),
                row.data(),
                static_cast<int>(num_columns),
                0.01f,
                &min,
                &max,
                "%.3f");
            ImGui::PopID();
            [[unlikely]] if (row_changed)
            {
                value.SetRow(row_index, row);
            }
            changed = changed || row_changed;
        }
        ImGui::TreePop();
    }
    return changed;
}

template <typename T>
bool MatrixProperty(edt::GUID type_guid, std::string_view name, void* address, bool& value_changed)
{
    constexpr auto type_info = cppreflection::GetStaticTypeInfo<T>();
    if (type_info.guid == type_guid)
    {
        T& member_ref = *reinterpret_cast<T*>(address);  // NOLINT
        value_changed |= MatrixProperty(name, member_ref);
        return true;
    }

    return false;
}

void EnsureHandledType(bool found_type, const edt::GUID& guid)
{
    [[likely]] if (found_type)
    {
        return;
    }
    const auto type_info = cppreflection::GetTypeRegistry()->FindType(guid);
    if (!type_info)
    {
        const auto char_arr = guid.ToCharArray();
        throw ErrorHandling::RuntimeErrorWithMessage(
            "Could not find a type with guid \"{}\", in the type registry",
            std::string_view{char_arr.data(), char_arr.size()});
    }
    throw ErrorHandling::RuntimeErrorWithMessage(
        "type type \"{}\" is not supported by simple type widget feature",
        type_info->GetName());
}

void SimpleTypeWidget(edt::GUID type_guid, std::string_view name, void* value, bool& value_changed)
{
    value_changed = false;
    const bool found_type = ScalarProperty<float>(type_guid, name, value, value_changed) ||
                            ScalarProperty<double>(type_guid, name, value, value_changed) ||
                            ScalarProperty<uint8_t>(type_guid, name, value, value_changed) ||
                            ScalarProperty<uint16_t>(type_guid, name, value, value_changed) ||
                            ScalarProperty<uint32_t>(type_guid, name, value, value_changed) ||
                            ScalarProperty<uint64_t>(type_guid, name, value, value_changed) ||
                            ScalarProperty<int8_t>(type_guid, name, value, value_changed) ||
                            ScalarProperty<int16_t>(type_guid, name, value, value_changed) ||
                            ScalarProperty<int32_t>(type_guid, name, value, value_changed) ||
                            ScalarProperty<int64_t>(type_guid, name, value, value_changed) ||
                            VectorProperty<Vec2f>(type_guid, name, value, value_changed) ||
                            VectorProperty<Vec3f>(type_guid, name, value, value_changed) ||
                            VectorProperty<Vec4f>(type_guid, name, value, value_changed) ||
                            MatrixProperty<Mat4f>(type_guid, name, value, value_changed);
    EnsureHandledType(found_type, type_guid);
}

void SimpleTypeWidget(edt::GUID type_guid, std::string_view name, const void* value)
{
    const bool found_type =
        ScalarProperty<float>(type_guid, name, value) || ScalarProperty<double>(type_guid, name, value) ||
        ScalarProperty<uint8_t>(type_guid, name, value) || ScalarProperty<uint16_t>(type_guid, name, value) ||
        ScalarProperty<uint32_t>(type_guid, name, value) || ScalarProperty<uint64_t>(type_guid, name, value) ||
        ScalarProperty<int8_t>(type_guid, name, value) || ScalarProperty<int16_t>(type_guid, name, value) ||
        ScalarProperty<int32_t>(type_guid, name, value) || ScalarProperty<int64_t>(type_guid, name, value);
    EnsureHandledType(found_type, type_guid);
}

void TypeIdWidget(edt::GUID type_guid, void* base, bool& value_changed)
{
    const cppreflection::Type* type_info = cppreflection::GetTypeRegistry()->FindType(type_guid);
    for (const cppreflection::Field* field : type_info->GetFields())
    {
        void* pmember = field->GetValue(base);
        bool member_changed = false;
        SimpleTypeWidget(field->GetType()->GetGuid(), field->GetName(), pmember, member_changed);
        value_changed |= member_changed;
    }
}

}  // namespace klgl
