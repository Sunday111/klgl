#include <imgui.h>

#include <EverydayTools/Math/Math.hpp>
#include <klgl/events/event_listener_interface.hpp>
#include <klgl/events/event_listener_method.hpp>
#include <klgl/events/event_manager.hpp>
#include <klgl/events/mouse_events.hpp>
#include <klgl/mesh/mesh_data.hpp>
#include <klgl/opengl/vertex_attribute_helper.hpp>
#include <klgl/shader/shader.hpp>
#include <klgl/template/register_attribute.hpp>

#include "klgl/application.hpp"
#include "klgl/camera/camera_2d.hpp"
#include "klgl/error_handling.hpp"
#include "klgl/opengl/gl_api.hpp"
#include "klgl/reflection/matrix_reflect.hpp"  // IWYU pragma: keep
#include "klgl/rendering/curve_renderer_2d.hpp"
#include "klgl/window.hpp"

namespace klgl::curve_example
{

using namespace edt::lazy_matrix_aliases;  // NOLINT

class CurveApp : public Application
{
    static constexpr edt::Vec4u8 kRed{255, 0, 0, 255};
    static constexpr size_t kSamplesPerSegment = 20;

    std::tuple<int, int> GetOpenGLVersion() const override { return {4, 3}; }

    void Initialize() override
    {
        Application::Initialize();

        event_listener_ = klgl::events::EventListenerMethodCallbacks<&CurveApp::OnMouseScroll>::CreatePtr(this);
        GetEventManager().AddEventListener(*event_listener_);

        OpenGl::SetClearColor({});
        GetWindow().SetSize(1000, 1000);
        GetWindow().SetTitle("Curve Example");

        control_points_ = edt::Math::GenerateSpiralPoints(100, {2, 2});

        for (size_t i = 0; i != control_points_.size(); ++i)
        {
            constexpr float m = 0.1f;
            float off = (1.f - m / 2) + static_cast<float>(i & 1) * m / 2;
            float len = control_points_[i].Length();
            Vec2f dir = control_points_[i] / len;
            control_points_[i] = dir * len * off;
        }

        {
            std::vector<CurveRenderer2d::ControlPoint> vertices;
            vertices.reserve(control_points_.size());
            for (size_t i = 0; i != control_points_.size(); ++i)
            {
                vertices.push_back(
                    {.position = control_points_[i],
                     .color = Vec4f(
                         edt::Math::GetRainbowColors(
                             10.f * static_cast<float>(i) / static_cast<float>(control_points_.size()))
                                 .Cast<float>() /
                             256,
                         1.f)});
            }

            curve_renderer_.SetPoints(vertices);
        }

        {
            std::vector<CurveRenderer2d::ControlPoint> vertices{
                {.position = {-1, -1}, .color = {1, 0, 0, 1}},
                {.position = {0, 1}, .color = {0, 1, 0, 0.5f}},
                {.position = {1, -1}, .color = {0, 0, 1, 0}},
            };
            curve_renderer_2_.SetPoints(vertices);
            curve_renderer_2_.thickness_ = 20.f;
        }

        OpenGl::EnableBlending();
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    void Tick() override
    {
        auto viewport = Viewport::FromWindowSize(GetWindow().GetSize());
        render_transforms.Update(camera, viewport, AspectRatioPolicy::ShrinkToFit);
        curve_renderer_.Draw(viewport.size.Cast<float>(), render_transforms.world_to_view);
        curve_renderer_2_.Draw(viewport.size.Cast<float>(), render_transforms.world_to_view);

        HandleInput();
    }

    void HandleInput()
    {
        if (!ImGui::GetIO().WantCaptureKeyboard)
        {
            int right = 0;
            int up = 0;
            if (ImGui::IsKeyDown(ImGuiKey_W)) up += 1;
            if (ImGui::IsKeyDown(ImGuiKey_S)) up -= 1;
            if (ImGui::IsKeyDown(ImGuiKey_D)) right += 1;
            if (ImGui::IsKeyDown(ImGuiKey_A)) right -= 1;
            if (std::abs(right) + std::abs(up))
            {
                edt::Vec2f delta{};
                delta += static_cast<float>(right) * edt::Vec2f::AxisX();
                delta += static_cast<float>(up) * edt::Vec2f::AxisY();
                camera.eye = camera.eye + delta * move_speed_ * GetLastFrameDurationSeconds() / camera.zoom;
            }
        }
    }

    void OnMouseScroll(const klgl::events::OnMouseScroll& event)
    {
        if (ImGui::GetIO().WantCaptureMouse) return;

        zoom_power_ += event.value.y();
        camera.zoom = std::max(std::pow(1.1f, zoom_power_), 0.1f);
    }

    std::unique_ptr<klgl::events::IEventListener> event_listener_;
    std::vector<Vec2f> control_points_;
    Camera2d camera{};
    RenderTransforms2d render_transforms{};

    CurveRenderer2d curve_renderer_;
    CurveRenderer2d curve_renderer_2_;

    // float line_width_ = 0.001f;
    float move_speed_ = 0.5f;
    float zoom_power_ = 0.f;
};

void Main()
{
    CurveApp app;
    app.Run();
}
}  // namespace klgl::curve_example

int main()
{
    klgl::ErrorHandling::InvokeAndCatchAll(klgl::curve_example::Main);
    return 0;
}
