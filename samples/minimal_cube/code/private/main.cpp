#include <imgui.h>

#include <EverydayTools/Math/Math.hpp>
#include <ass/enum_map.hpp>
#include <ass/enum_set.hpp>

#include "klgl/application.hpp"
#include "klgl/camera/camera3d.hpp"
#include "klgl/error_handling.hpp"
#include "klgl/mesh/mesh_data.hpp"
#include "klgl/mesh/procedural_mesh_generator.hpp"
#include "klgl/opengl/gl_api.hpp"
#include "klgl/shader/shader.hpp"
#include "klgl/ui/simple_type_widget.hpp"
#include "klgl/window.hpp"

class CubeApp : public klgl::Application
{
    void Initialize() override
    {
        klgl::Application::Initialize();

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
            edt::Vec3f::Size(),
            klgl::GlVertexAttribComponentType::Float,
            false,
            sizeof(edt::Vec3f),
            nullptr);

        // Load shader
        shader_ = std::make_unique<klgl::Shader>("just_color.shader.json");

        {
            const auto m = edt::Math::RotationMatrix3dZ(edt::Math::DegToRad(135.f))
                               .MatMul(edt::Math::RotationMatrix3dX(edt::Math::DegToRad(-45.f)));
            camera_.eye_ = edt::Vec3f{3, 3, 4};
            camera_.dir_ = edt::Math::TransformVector(m, edt::Vec3f{0.f, 1.f, 0.f});
            camera_.right_ = edt::Math::TransformVector(m, edt::Vec3f{1.f, 0.f, 0.f});
        }

        klgl::OpenGl::EnableFaceCulling(true);
        klgl::OpenGl::CullFace(klgl::GlCullFaceMode::Back);
        // klgl::OpenGl::PolygonMode(klgl::GlPolygonMode::Line);
    }

    void RenderWorld()
    {
        shader_->Use();

        const float cube_rot = GetTimeSeconds();
        const edt::Mat4f model =
            edt::Math::RotationMatrix3dZ(cube_rot).MatMul(edt::Math::ScaleMatrix(edt::Vec3f{} + 0.5f));
        shader_->SetUniform(u_model_, model);
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
        }
        ImGui::End();
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
        if (ImGui::GetIO().WantCaptureKeyboard) return;

        int right = 0;
        int forward = 0;
        int up = 0;
        if (ImGui::IsKeyDown(ImGuiKey_W)) forward += 1;
        if (ImGui::IsKeyDown(ImGuiKey_A)) right -= 1;
        if (ImGui::IsKeyDown(ImGuiKey_S)) forward -= 1;
        if (ImGui::IsKeyDown(ImGuiKey_D)) right += 1;
        if (ImGui::IsKeyDown(ImGuiKey_E)) up += 1;
        if (ImGui::IsKeyDown(ImGuiKey_Q)) up -= 1;
        if (right + forward + up)
        {
            const float k = move_speed_ * GetLastFrameDurationSeconds();
            camera_.eye_ += camera_.dir_ * k * static_cast<float>(forward);
            camera_.eye_ += camera_.right_ * k * static_cast<float>(right);
            camera_.eye_ += camera_.GetUp() * k * static_cast<float>(up);
        }
    }

    klgl::Camera3d camera_{};
    float move_speed_ = 5.f;

    klgl::UniformHandle u_color_ = klgl::UniformHandle("u_color");
    klgl::UniformHandle u_model_ = klgl::UniformHandle("u_model");
    klgl::UniformHandle u_view_ = klgl::UniformHandle("u_view");
    klgl::UniformHandle u_projection_ = klgl::UniformHandle("u_projection");

    std::shared_ptr<klgl::Shader> shader_;
    std::shared_ptr<klgl::MeshOpenGL> mesh_;
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
