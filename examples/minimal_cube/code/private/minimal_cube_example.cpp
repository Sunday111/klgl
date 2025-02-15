#include <imgui.h>

#include <EverydayTools/Math/Math.hpp>
#include <ass/enum_map.hpp>
#include <ass/enum_set.hpp>

#include "klgl/application.hpp"
#include "klgl/camera/camera_3d.hpp"
#include "klgl/error_handling.hpp"
#include "klgl/events/event_listener_method.hpp"
#include "klgl/events/event_manager.hpp"
#include "klgl/events/mouse_events.hpp"
#include "klgl/math/transform.hpp"
#include "klgl/mesh/mesh_data.hpp"
#include "klgl/mesh/procedural_mesh_generator.hpp"
#include "klgl/opengl/gl_api.hpp"
#include "klgl/shader/shader.hpp"
#include "klgl/ui/simple_type_widget.hpp"
#include "klgl/window.hpp"

using namespace edt::lazy_matrix_aliases;  // NOLINT

class CubeApp : public klgl::Application
{
    void Initialize() override
    {
        klgl::Application::Initialize();

        event_listener_ = klgl::events::EventListenerMethodCallbacks<&CubeApp::OnMouseMove>::CreatePtr(this);
        GetEventManager().AddEventListener(*event_listener_);

        klgl::OpenGl::SetClearColor({});
        GetWindow().SetSize(1000, 1000);
        GetWindow().SetTitle("Cube");

        // Create quad mesh
        const auto mesh_data = klgl::ProceduralMeshGenerator::GenerateCubeMesh();
        mesh_ = klgl::MeshOpenGL::MakeFromData(
            std::span{mesh_data.vertices},
            std::span{mesh_data.indices},
            mesh_data.topology);
        mesh_->Bind();

        // Vertex buffer attributes
        klgl::OpenGl::EnableVertexAttribArray(0);
        klgl::OpenGl::VertexAttribPointer(
            0,
            Vec3f::Size(),
            klgl::GlVertexAttribComponentType::Float,
            false,
            sizeof(Vec3f),
            nullptr);

        // Load shader
        shader_ = std::make_unique<klgl::Shader>("just_color_3d");

        klgl::OpenGl::EnableFaceCulling(true);
        klgl::OpenGl::CullFace(klgl::GlCullFaceMode::Back);
    }

    void RenderWorld()
    {
        shader_->Use();
        shader_->SetUniform(u_model_, cube_transform_.Matrix().Transposed());
        shader_->SetUniform(u_view_, camera_.GetViewMatrix());
        shader_->SetUniform(u_projection_, camera_.GetProjectionMatrix(GetWindow().GetAspect()));
        shader_->SetUniform(u_color_, edt::Math::GetRainbowColorsA(GetTimeSeconds()).Cast<float>() / 255.f);
        shader_->SendUniforms();
        mesh_->BindAndDraw();
    }

    void RenderGUI()
    {
        if (ImGui::Begin("Settings"))
        {
            camera_.Widget();
            ImGui::Separator();
            klgl::SimpleTypeWidget("move_speed", move_speed_);

            if (ImGui::CollapsingHeader("cube"))
            {
                cube_transform_.Widget();
            }
        }
        ImGui::End();

        shader_->DrawDetails();
    }

    void Tick() override
    {
        klgl::Application::Tick();
        HandleInput();
        RenderWorld();
        RenderGUI();
    }

    void OnMouseMove(const klgl::events::OnMouseMove& event)
    {
        constexpr float sensitivity = 0.01f;
        if (GetWindow().IsFocused() && GetWindow().IsInInputMode() && !ImGui::GetIO().WantCaptureMouse)
        {
            const auto delta = (event.current - event.previous) * sensitivity;
            const auto [yaw, pitch, roll] = camera_.GetRotation();
            camera_.SetRotation({yaw + delta.x(), pitch + delta.y(), roll});
        }
    }

    void HandleInput()
    {
        if (!ImGui::GetIO().WantCaptureKeyboard)
        {
            int right = 0;
            int forward = 0;
            int up = 0;
            if (ImGui::IsKeyDown(ImGuiKey_W)) forward += 1;
            if (ImGui::IsKeyDown(ImGuiKey_S)) forward -= 1;
            if (ImGui::IsKeyDown(ImGuiKey_D)) right += 1;
            if (ImGui::IsKeyDown(ImGuiKey_A)) right -= 1;
            if (ImGui::IsKeyDown(ImGuiKey_E)) up += 1;
            if (ImGui::IsKeyDown(ImGuiKey_Q)) up -= 1;
            if (std::abs(right) + std::abs(forward) + std::abs(up))
            {
                Vec3f delta = static_cast<float>(forward) * camera_.GetForwardAxis();
                delta += static_cast<float>(right) * camera_.GetRightAxis();
                delta += static_cast<float>(up) * camera_.GetUpAxis();
                camera_.SetEye(camera_.GetEye() + delta * move_speed_ * GetLastFrameDurationSeconds());
            }
        }
    }

    std::unique_ptr<klgl::events::IEventListener> event_listener_;

    klgl::UniformHandle u_color_ = klgl::UniformHandle("u_color");
    klgl::UniformHandle u_model_ = klgl::UniformHandle("u_model");
    klgl::UniformHandle u_view_ = klgl::UniformHandle("u_view");
    klgl::UniformHandle u_projection_ = klgl::UniformHandle("u_projection");

    std::shared_ptr<klgl::Shader> shader_;
    std::shared_ptr<klgl::MeshOpenGL> mesh_;

    // Camera (3, 3, 4) looks at the cube (6, 6, 0).
    float move_speed_ = 5.f;
    klgl::Transform cube_transform_{.translation = {6, 6, 0}};
    klgl::Camera3d camera_{Vec3f{3, 3, 4}, {.yaw = 45, .pitch = 45}};
};

void Main()
{
    CubeApp app;
    app.Run();
}

int main()
{
    klgl::ErrorHandling::InvokeAndCatchAll(Main);
    return 0;
}
