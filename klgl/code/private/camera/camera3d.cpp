#include "klgl/camera/camera3d.hpp"

#include <imgui.h>

#include "klgl/reflection/matrix_reflect.hpp"  // IWYU pragma: keep
#include "klgl/ui/simple_type_widget.hpp"

namespace klgl
{

bool Camera3d::Widget()
{
    bool updated = false;
    if (!ImGui::CollapsingHeader("Camera")) return updated;

    updated |= SimpleTypeWidget("eye", eye_);
    updated |= SimpleTypeWidget("dir", dir_);
    updated |= SimpleTypeWidget("right", right_);

    auto up = GetUp();
    SimpleTypeWidget("up", up);

    ImGui::Separator();
    updated |= SimpleTypeWidget("near", near_);
    updated |= SimpleTypeWidget("far", far_);
    updated |= SimpleTypeWidget("fov", fov_);

    return updated;
}
}  // namespace klgl
