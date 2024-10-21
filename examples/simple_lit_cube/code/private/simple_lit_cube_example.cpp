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
#include "klgl/template/register_attribute.hpp"
#include "klgl/ui/simple_type_widget.hpp"
#include "klgl/window.hpp"

using namespace edt::lazy_matrix_aliases;  // NOLINT

struct Vertex
{
    edt::Vec3f position;
    edt::Vec3f normal;
};

class SimpleLitCubeApp : public klgl::Application
{
    void Initialize() override
    {
        klgl::Application::Initialize();

        event_listener_ = klgl::events::EventListenerMethodCallbacks<&SimpleLitCubeApp::OnMouseMove>::CreatePtr(this);
        GetEventManager().AddEventListener(*event_listener_);

        klgl::OpenGl::SetClearColor({});
        GetWindow().SetSize(1000, 1000);
        GetWindow().SetTitle("Cube");

        shader_ = std::make_unique<klgl::Shader>("basic_light_3d");
        a_position_ = shader_->GetInfo().VerifyAndGetVertexAttributeLocation<edt::Vec3f>("a_position");
        a_normal_ = shader_->GetInfo().VerifyAndGetVertexAttributeLocation<edt::Vec3f>("a_normal");

        // Create quad mesh
        const auto mesh_data = klgl::ProceduralMeshGenerator::GenerateCubeMesh();

        std::vector<Vertex> vertices(mesh_data.vertices.size());
        for (size_t i = 0; i != vertices.size(); ++i)
        {
            auto& vertex = vertices[i];
            vertex.position = mesh_data.vertices[i];
            vertex.normal = mesh_data.normals[i];
        }

        mesh_ = klgl::MeshOpenGL::MakeFromData(std::span{vertices}, std::span{mesh_data.indices}, mesh_data.topology);
        mesh_->Bind();

        klgl::RegisterAttribute<&Vertex::position>(a_position_);
        klgl::RegisterAttribute<&Vertex::normal>(a_normal_);

        klgl::OpenGl::EnableDepthTest();
        klgl::OpenGl::EnableFaceCulling(true);
        klgl::OpenGl::CullFace(klgl::GlCullFaceMode::Front);

        for (int x = -10; x != 11; ++x)
        {
            for (int y = -10; y != 11; ++y)
            {
                cubes_.push_back(klgl::Transform{
                    .translation = edt::Vec3i{x, y, 0}.Cast<float>(),
                    .scale = {0.3f, 0.3f, 0.3f},
                }
                                     .Matrix()
                                     .Transposed());
            }
        }
    }

    void RenderWorld()
    {
        shader_->Use();
        shader_->SetUniform(u_ambient_, 0.1f);
        shader_->SetUniform(u_specular_, 0.5f);
        shader_->SetUniform(u_view_pos_, camera_.GetEye());
        shader_->SetUniform(u_object_color_, edt::Vec4f{1, 0, 0, 1});
        shader_->SetUniform(u_light_pos_, edt::Vec3f{9, 9, 3});
        shader_->SetUniform(u_light_color_, edt::Vec3f{1, 1, 1});
        shader_->SetUniform(u_view_, camera_.GetViewMatrix());
        shader_->SetUniform(u_projection_, camera_.GetProjectionMatrix(GetWindow().GetAspect()));

        for (const auto& cube : cubes_)
        {
            shader_->SetUniform(u_model_, cube);
            shader_->SendUniforms();
            mesh_->BindAndDraw();
        }
    }

    void RenderGUI()
    {
        if (ImGui::Begin("Settings"))
        {
            camera_.Widget();
            ImGui::Separator();
            klgl::SimpleTypeWidget("move_speed", move_speed_);
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

    std::unique_ptr<klgl::events::IEventListener> event_listener_;

    size_t a_position_{};
    size_t a_normal_{};
    klgl::UniformHandle u_view_pos_ = klgl::UniformHandle("u_view_pos");
    klgl::UniformHandle u_light_pos_ = klgl::UniformHandle("u_light_pos");
    klgl::UniformHandle u_ambient_ = klgl::UniformHandle("u_ambient");
    klgl::UniformHandle u_specular_ = klgl::UniformHandle("u_specular");
    klgl::UniformHandle u_object_color_ = klgl::UniformHandle("u_object_color");
    klgl::UniformHandle u_model_ = klgl::UniformHandle("u_model");
    klgl::UniformHandle u_view_ = klgl::UniformHandle("u_view");
    klgl::UniformHandle u_projection_ = klgl::UniformHandle("u_projection");
    klgl::UniformHandle u_light_color_ = klgl::UniformHandle("u_light_color");

    std::shared_ptr<klgl::Shader> shader_;
    std::shared_ptr<klgl::MeshOpenGL> mesh_;

    float move_speed_ = 5.f;
    std::vector<edt::Mat4f> cubes_;
    klgl::Camera3d camera_{Vec3f{3, 3, 4}, {.yaw = 45, .pitch = 45}};
};

void Main()
{
    SimpleLitCubeApp app;
    app.Run();
}

int main()
{
    klgl::ErrorHandling::InvokeAndCatchAll(Main);
    return 0;
}
