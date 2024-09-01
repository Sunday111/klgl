#include "klgl/math/transform.hpp"

#include <imgui.h>

#include "klgl/ui/simple_type_widget.hpp"

namespace klgl
{

void Transform::Widget()
{
    if (!ImGui::CollapsingHeader("Transform")) return;

    SimpleTypeWidget("translation", translation);
    SimpleTypeWidget("yaw", rotation.yaw);
    SimpleTypeWidget("pitch", rotation.pitch);
    SimpleTypeWidget("roll", rotation.roll);
    SimpleTypeWidget("scale", scale);
}
}  // namespace klgl
