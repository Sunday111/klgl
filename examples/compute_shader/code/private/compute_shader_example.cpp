#include <imgui.h>

#include <EverydayTools/Math/Math.hpp>
#include <ass/enum_map.hpp>
#include <ass/enum_set.hpp>
#include <klgl/shader/shader.hpp>

#include "klgl/application.hpp"
#include "klgl/camera/camera_3d.hpp"
#include "klgl/error_handling.hpp"
#include "klgl/events/event_listener_method.hpp"
#include "klgl/events/event_manager.hpp"
#include "klgl/events/mouse_events.hpp"
#include "klgl/opengl/gl_api.hpp"
#include "klgl/shader/shader.hpp"
#include "klgl/window.hpp"

class Painter2dApp : public klgl::Application
{
    void Initialize() override
    {
        klgl::Application::Initialize();
        klgl::OpenGl::SetClearColor({});
        GetWindow().SetSize(1000, 1000);
        GetWindow().SetTitle("Painter 2d");
        compute_shader_ = std::make_unique<klgl::Shader>("compute_shader");
        just_color_ = std::make_unique<klgl::Shader>("just_color_3d");

        event_listener_ = klgl::events::EventListenerMethodCallbacks<&Painter2dApp::OnMouseMove>::CreatePtr(this);
        GetEventManager().AddEventListener(*event_listener_);

        std::vector<edt::Vec4f> positions = CalcPositions();

        glCreateBuffers(1, &m_posBuf);
        glCreateBuffers(1, &m_velBuf);

        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_posBuf);
        klgl::OpenGl::BufferData(klgl::GlBufferType::ShaderStorage, std::span{positions}, klgl::GlUsage::DynamicDraw);

        std::vector<edt::Vec4f> velocities(positions.size());
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_velBuf);
        klgl::OpenGl::BufferData(klgl::GlBufferType::ShaderStorage, std::span{velocities}, klgl::GlUsage::DynamicCopy);

        glGenVertexArrays(1, &m_vao);
        glBindVertexArray(m_vao);

        glBindBuffer(GL_ARRAY_BUFFER, m_posBuf);
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, nullptr);
        glEnableVertexAttribArray(0);

        glBindVertexArray(0);

        glCreateBuffers(1, &m_bhBuf);
        glBindBuffer(GL_ARRAY_BUFFER, m_bhBuf);
        std::array<float, 8> data{m_bh1.x(), m_bh1.y(), m_bh1.z(), 1, m_bh2.x(), m_bh2.y(), m_bh2.z(), 1};
        klgl::OpenGl::BufferData(klgl::GlBufferType::Array, std::span{data}, klgl::GlUsage::DynamicDraw);

        glGenVertexArrays(1, &m_bhVao);
        glBindVertexArray(m_bhVao);

        glBindBuffer(GL_ARRAY_BUFFER, m_bhBuf);
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, nullptr);
        glEnableVertexAttribArray(0);

        glBindVertexArray(0);
    }

    std::vector<edt::Vec4f> CalcPositions()
    {
        std::vector<edt::Vec4f> positions(total_particles_);
        edt::Vec3f delta(2.f / particles_per_axis_.Cast<float>());

        size_t i = 0;
        for (size_t x = 0; x < particles_per_axis_.x(); x++)
        {
            for (size_t y = 0; y < particles_per_axis_.y(); y++)
            {
                for (size_t z = 0; z < particles_per_axis_.z(); z++)
                {
                    positions[i] = edt::Vec4f((edt::Vec3<size_t>{x, y, z}.Cast<float>() * delta - 1), 1.f);
                    i++;
                }
            }
        }

        return positions;
    }

    void Tick() override
    {
        m_angle += m_speed * GetLastFrameDurationSeconds();
        // printf("%f\n", m_angle);
        if (m_angle > 360.0f)
        {
            m_angle -= 360.0f;
        }

        {
            compute_shader_->Use();

            auto mat = edt::Math::RotationMatrix3dY(edt::Math::DegToRad(m_angle));
            compute_shader_->SetUniform(bhu1, edt::Math::TransformPos(mat, m_bh1));
            compute_shader_->SetUniform(bhu2, edt::Math::TransformPos(mat, m_bh2));

            glDispatchCompute(total_particles_, 1, 1);
            glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
        }

        {
            just_color_->Use();
            just_color_->SetUniform(u_model_, edt::Mat4f::Identity());
            just_color_->SetUniform(u_view_, camera_.GetViewMatrix());
            just_color_->SetUniform(u_projection_, camera_.GetProjectionMatrix(GetWindow().GetAspect()));
            just_color_->SetUniform(u_color_, edt::Math::GetRainbowColorsA(GetTimeSeconds()).Cast<float>() / 255.f);
            just_color_->SendUniforms();

            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            // Draw the particles
            glPointSize(2.0f);
            glBindVertexArray(m_vao);
            glDrawArrays(GL_POINTS, 0, total_particles_);
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
                edt::Vec3f delta = static_cast<float>(forward) * camera_.GetForwardAxis();
                delta += static_cast<float>(right) * camera_.GetRightAxis();
                delta += static_cast<float>(up) * camera_.GetUpAxis();
                camera_.SetEye(camera_.GetEye() + delta * move_speed_ * GetLastFrameDurationSeconds());
            }
        }
    }

    static constexpr edt::Vec3<size_t> particles_per_axis_{10, 10, 10};
    static constexpr size_t total_particles_ =
        particles_per_axis_.x() * particles_per_axis_.y() * particles_per_axis_.z();

    float m_speed = 35;
    float m_angle = 0;

    GLuint m_vao = 0;
    GLuint m_posBuf = 0;
    GLuint m_velBuf = 0;

    GLuint m_bhVao = 0;
    GLuint m_bhBuf = 0;

    edt::Vec3f m_bh1{5, 0, 0};
    edt::Vec3f m_bh2{-5, 0, 0};
    std::shared_ptr<klgl::Shader> compute_shader_;
    std::shared_ptr<klgl::Shader> just_color_;

    float move_speed_ = 5.f;
    klgl::UniformHandle bhu1 = klgl::UniformHandle("BlackHolePos1");
    klgl::UniformHandle bhu2 = klgl::UniformHandle("BlackHolePos2");
    klgl::UniformHandle u_color_ = klgl::UniformHandle("u_color");
    klgl::UniformHandle u_model_ = klgl::UniformHandle("u_model");
    klgl::UniformHandle u_view_ = klgl::UniformHandle("u_view");
    klgl::UniformHandle u_projection_ = klgl::UniformHandle("u_projection");
    klgl::Camera3d camera_{edt::Vec3f{3, 3, 4}, {.yaw = 45, .pitch = 45}};

    std::unique_ptr<klgl::events::IEventListener> event_listener_;
};

void Main()
{
    Painter2dApp app;
    app.Run();
}

int main()
{
    klgl::ErrorHandling::InvokeAndCatchAll(Main);
    return 0;
}
