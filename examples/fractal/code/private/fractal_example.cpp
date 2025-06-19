#include <fmt/format.h>
#include <imgui.h>

#include <EverydayTools/Math/Math.hpp>
#include <klgl/events/event_listener_interface.hpp>
#include <klgl/events/event_listener_method.hpp>
#include <klgl/events/event_manager.hpp>
#include <klgl/events/mouse_events.hpp>
#include <klgl/ui/simple_type_widget.hpp>

#include "klgl/application.hpp"
#include "klgl/camera/camera_2d.hpp"
#include "klgl/error_handling.hpp"
#include "klgl/mesh/mesh_data.hpp"
#include "klgl/mesh/procedural_mesh_generator.hpp"
#include "klgl/opengl/gl_api.hpp"
#include "klgl/reflection/matrix_reflect.hpp"  // IWYU pragma: keep
#include "klgl/shader/shader.hpp"
#include "klgl/window.hpp"

struct FractalParams
{
    float a = 0.7785f;
    float b = 0.7785f;
    float c = 0.3f;
    float d = 0.5f;
    float time = 0.0f;
    bool use_current_time = false;

    edt::Vec2f MakeJuliaConstant(float current_time)
    {
        if (use_current_time)
        {
            time = current_time;
        }

        return edt::Vec2f{a * std::cos(time * c), b * std::sin(time * d)};
    }
};

class FractalApp : public klgl::Application
{
    void Initialize() override
    {
        klgl::Application::Initialize();
        event_listener_ = klgl::events::EventListenerMethodCallbacks<&FractalApp::OnMouseScroll>::CreatePtr(this);
        GetEventManager().AddEventListener(*event_listener_);

        klgl::OpenGl::SetClearColor({});
        GetWindow().SetSize(1000, 1000);
        GetWindow().SetTitle("Fractal");
        SetTargetFramerate(30.f);

        // Create quad mesh
        const auto mesh_data = klgl::ProceduralMeshGenerator::GenerateQuadMesh();
        mesh_ = klgl::MeshOpenGL::MakeFromData(
            std::span{mesh_data.vertices},
            std::span{mesh_data.indices},
            mesh_data.topology);
        mesh_->Bind();

        // Vertex buffer attributes
        klgl::OpenGl::EnableVertexAttribArray(0);
        klgl::OpenGl::VertexAttribPointer(
            0,
            edt::Vec2f::Size(),
            klgl::GlVertexAttribComponentType::Float,
            false,
            sizeof(edt::Vec2f),
            nullptr);

        // Load shader
        shader_ = std::make_unique<klgl::Shader>("fractal");
        shader_->Use();
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
                camera_.eye = camera_.eye + delta * move_speed_ * GetLastFrameDurationSeconds() / camera_.zoom;
            }
        }
    }

    void Tick() override
    {
        klgl::Application::Tick();

        const auto resolution = GetWindow().GetSize2f();

        HandleInput();
        viewport_.MatchWindowSize(resolution);
        render_transforms_.Update(camera_, viewport_);

        shader_->Use();
        shader_->SetUniform(u_screen_to_world_, render_transforms_.screen_to_world.Transposed());
        // shader_->SetUniform(u_time_, GetTimeSeconds());
        shader_->SetUniform(u_julia_constant, fractal_params_.MakeJuliaConstant(GetTimeSeconds()));
        shader_->SendUniforms();

        mesh_->BindAndDraw();

        DrawGUI();
    }

    void OnMouseScroll(const klgl::events::OnMouseScroll& event)
    {
        if (!ImGui::GetIO().WantCaptureMouse)
        {
            zoom_power_ += event.value.y();
            camera_.zoom = std::max(std::pow(1.1f, zoom_power_), 0.1f);
        }
    }

    void DrawGUI()
    {
        if (ImGui::Begin("Settings"))
        {
            if (ImGui::CollapsingHeader("Julia constant"))
            {
                ImGui::SliderFloat("a", &fractal_params_.a, 0.001f, 1.f);
                ImGui::SliderFloat("b", &fractal_params_.b, 0.001f, 1.f);
                ImGui::SliderFloat("c", &fractal_params_.c, 0.001f, 1.f);
                ImGui::SliderFloat("d", &fractal_params_.d, 0.001f, 1.f);
                ImGui::Checkbox("use current time", &fractal_params_.use_current_time);
            }

            ImGui::End();
        }
    }

    klgl::UniformHandle u_screen_to_world_ = klgl::UniformHandle("u_screen_to_world");
    klgl::UniformHandle u_julia_constant = klgl::UniformHandle("u_julia_constant");
    // klgl::UniformHandle u_time_ = klgl::UniformHandle("u_time");

    std::unique_ptr<klgl::events::IEventListener> event_listener_;
    std::shared_ptr<klgl::Shader> shader_;
    std::shared_ptr<klgl::MeshOpenGL> mesh_;
    float zoom_power_ = 0.f;
    float move_speed_ = 0.5f;
    klgl::Camera2d camera_;
    klgl::RenderTransforms2d render_transforms_;
    klgl::Viewport viewport_;
    FractalParams fractal_params_;
};

void Main()
{
    FractalApp app;
    app.Run();
}

int main()
{
    klgl::ErrorHandling::InvokeAndCatchAll(Main);
    return 0;
}
