#include <imgui.h>

#include <EverydayTools/Math/Math.hpp>
#include <ass/enum_map.hpp>
#include <ass/enum_set.hpp>
#include <klgl/shader/shader.hpp>
#include <klgl/ui/simple_type_widget.hpp>

#include "klgl/application.hpp"
#include "klgl/camera/camera_3d.hpp"
#include "klgl/error_handling.hpp"
#include "klgl/events/event_listener_method.hpp"
#include "klgl/events/event_manager.hpp"
#include "klgl/events/mouse_events.hpp"
#include "klgl/opengl/gl_api.hpp"
#include "klgl/reflection/matrix_reflect.hpp"  // IWYU pragma: keep
#include "klgl/shader/shader.hpp"
#include "klgl/window.hpp"

namespace klgl::compute_shader_example
{

using namespace edt::lazy_matrix_aliases;  // NOLINT
using Math = edt::Math;

class Painter2dApp : public Application
{
    std::tuple<int, int> GetOpenGLVersion() const override { return {4, 5}; }

    void Initialize() override
    {
        Application::Initialize();
        OpenGl::SetClearColor({});
        GetWindow().SetSize(1000, 1000);
        GetWindow().SetTitle("Painter 2d");
        SetTargetFramerate({});
        compute_shader_ = std::make_unique<Shader>("compute_shader");
        color_shader_ = std::make_unique<Shader>("just_color_3d");

        event_listener_ = events::EventListenerMethodCallbacks<&Painter2dApp::OnMouseMove>::CreatePtr(this);
        GetEventManager().AddEventListener(*event_listener_);

        std::vector<Vec4f> positions = CalcPositions();

        particels_positions_buffer_ = OpenGl::GenBuffer();
        particles_velocities_buffer_ = OpenGl::GenBuffer();

        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, particels_positions_buffer_.GetValue());
        OpenGl::BufferData(GlBufferType::ShaderStorage, std::span{positions}, GlUsage::DynamicDraw);

        std::vector<Vec4f> velocities(positions.size());
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, particles_velocities_buffer_.GetValue());
        OpenGl::BufferData(GlBufferType::ShaderStorage, std::span{velocities}, GlUsage::DynamicCopy);

        particles_vao_ = OpenGl::GenVertexArray();
        OpenGl::BindVertexArray(particles_vao_);
        OpenGl::BindBuffer(GlBufferType::Array, particels_positions_buffer_);
        OpenGl::VertexAttribPointer(0, 4, GlVertexAttribComponentType::Float, false, 0, nullptr);
        OpenGl::EnableVertexAttribArray(0);

        OpenGl::BindVertexArray({});

        bodies_positions_buffer_ = OpenGl::GenBuffer();
        OpenGl::BindBuffer(GlBufferType::Array, bodies_positions_buffer_);
        constexpr std::array data{kBodyAStartPosition, kBodyBStartPosition};
        OpenGl::BufferData(GlBufferType::Array, std::span{data}, GlUsage::DynamicDraw);

        bodies_vao_ = OpenGl::GenVertexArray();
        OpenGl::BindVertexArray(bodies_vao_);
        OpenGl::BindBuffer(GlBufferType::Array, bodies_positions_buffer_);
        OpenGl::VertexAttribPointer(0, 3, GlVertexAttribComponentType::Float, false, 0, nullptr);
        OpenGl::EnableVertexAttribArray({});

        OpenGl::BindVertexArray({});
    }

    static std::vector<Vec4f> CalcPositions()
    {
        std::vector<Vec4f> positions(kTotalParticles);

        const size_t s = static_cast<size_t>(std::round(std::pow(static_cast<float>(kTotalParticles), 1.f / 3)));
        auto delta = Vec3f{} + 2.f / static_cast<float>(s);

        [&]
        {
            size_t i = 0;
            for (size_t x = 0; x < s; x++)
            {
                for (size_t y = 0; y < s; y++)
                {
                    for (size_t z = 0; z < s; z++)
                    {
                        positions[i] = Vec4f((Vec3<size_t>{x, y, z}.Cast<float>() * delta - 1), 1.f);
                        if (++i == kTotalParticles)
                        {
                            return;
                        }
                    }
                }
            }
        }();

        return positions;
    }

    void Tick() override
    {
        HandleInput();

        const float delta_t = GetLastFrameDurationSeconds() * 0.01f;
        angle_ = std::fmod(angle_ + rotation_speed_ * delta_t, 360.f);

        const auto body_rotation = Math::RotationMatrix3dY(Math::DegToRad(angle_));
        const std::array bodies_positions{
            Math::TransformPos(body_rotation, kBodyAStartPosition),
            Math::TransformPos(body_rotation, kBodyBStartPosition)};

        // Compute particles
        compute_shader_->Use();
        compute_shader_->SetUniform(u_body_a_pos_, bodies_positions[0]);
        compute_shader_->SetUniform(u_body_b_pos_, bodies_positions[1]);
        compute_shader_->SetUniform(u_delta_t_, delta_t);
        compute_shader_->SendUniforms();
        glDispatchCompute(kTotalParticles, 1, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

        OpenGl::EnableBlending();
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        color_shader_->Use();
        color_shader_->SetUniform(u_model_, Mat4f::Identity());
        color_shader_->SetUniform(u_view_, camera_.GetViewMatrix());
        color_shader_->SetUniform(u_projection_, camera_.GetProjectionMatrix(GetWindow().GetAspect()));
        color_shader_->SetUniform(u_color_, Vec4f{1, 1, 1, 0.1f});
        color_shader_->SendUniforms();

        // Draw the particles
        OpenGl::PointSize(2.f);
        OpenGl::BindVertexArray(particles_vao_);
        OpenGl::DrawArrays(GlPrimitiveType::Points, 0, kTotalParticles);

        // Draw black holes
        color_shader_->SetUniform(u_color_, Vec4f{1, 0, 0, 1});
        color_shader_->SendUniforms();
        OpenGl::PointSize(15.f);

        OpenGl::BindBuffer(GlBufferType::Array, bodies_positions_buffer_);
        OpenGl::BufferSubData(GlBufferType::Array, 0, std::span{bodies_positions});
        OpenGl::BindVertexArray(bodies_vao_);
        OpenGl::DrawArrays(GlPrimitiveType::Points, 0, 2);

        RenderGUI();
    }

    void OnMouseMove(const events::OnMouseMove& event)
    {
        constexpr float sensitivity = 0.01f;
        if (GetWindow().IsFocused() && GetWindow().IsInInputMode() && !ImGui::GetIO().WantCaptureMouse)
        {
            const auto delta = (event.current - event.previous) * sensitivity;
            const auto [yaw, pitch, roll] = camera_.GetRotation();
            camera_.SetRotation({yaw + delta.x(), pitch + delta.y(), roll});
        }
    }

    void RenderGUI()
    {
        if (ImGui::Begin("Settings"))
        {
            camera_.Widget();
            ImGui::Separator();
            SimpleTypeWidget("Camera speed", camera_speed_);
            const auto framerate = static_cast<size_t>(GetFramerate());
            SimpleTypeWidget("Framerate", framerate);
        }
        ImGui::End();
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
                camera_.SetEye(camera_.GetEye() + delta * camera_speed_ * GetLastFrameDurationSeconds());
            }
        }
    }

    static constexpr size_t kTotalParticles = 1'000'000;
    static constexpr Vec3f kBodyAStartPosition{5, 0, 0};
    static constexpr Vec3f kBodyBStartPosition{-5, 0, 0};

    float rotation_speed_ = 700;
    float angle_ = 0;
    float camera_speed_ = 5.f;
    Camera3d camera_{Vec3f{0, 15, 0}, {.yaw = -90, .pitch = 0}};

    GlVertexArrayId particles_vao_;
    GlBufferId particels_positions_buffer_;
    GlBufferId particles_velocities_buffer_;

    GlVertexArrayId bodies_vao_;
    GlBufferId bodies_positions_buffer_;

    std::shared_ptr<Shader> compute_shader_;
    std::shared_ptr<Shader> color_shader_;

    UniformHandle u_body_a_pos_ = UniformHandle("BlackHolePos1");
    UniformHandle u_body_b_pos_ = UniformHandle("BlackHolePos2");
    UniformHandle u_delta_t_ = UniformHandle("u_delta_t");
    UniformHandle u_color_ = UniformHandle("u_color");
    UniformHandle u_model_ = UniformHandle("u_model");
    UniformHandle u_view_ = UniformHandle("u_view");
    UniformHandle u_projection_ = UniformHandle("u_projection");

    std::unique_ptr<events::IEventListener> event_listener_;
};

void Main()
{
    Painter2dApp app;
    app.Run();
}
}  // namespace klgl::compute_shader_example

int main()
{
    klgl::ErrorHandling::InvokeAndCatchAll(klgl::compute_shader_example::Main);
    return 0;
}
