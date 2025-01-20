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

class Painter2dApp : public klgl::Application
{
    std::tuple<int, int> GetOpenGLVersion() const override { return {4, 5}; }

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

        m_posBuf = klgl::OpenGl::GenBuffer();
        m_velBuf = klgl::OpenGl::GenBuffer();

        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_posBuf.GetValue());
        klgl::OpenGl::BufferData(klgl::GlBufferType::ShaderStorage, std::span{positions}, klgl::GlUsage::DynamicDraw);

        std::vector<edt::Vec4f> velocities(positions.size());
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_velBuf.GetValue());
        klgl::OpenGl::BufferData(klgl::GlBufferType::ShaderStorage, std::span{velocities}, klgl::GlUsage::DynamicCopy);

        glGenVertexArrays(1, &m_vao);
        glBindVertexArray(m_vao);

        glBindBuffer(GL_ARRAY_BUFFER, m_posBuf.GetValue());
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, nullptr);
        glEnableVertexAttribArray(0);

        glBindVertexArray(0);

        m_bhBuf = klgl::OpenGl::GenBuffer();
        glBindBuffer(GL_ARRAY_BUFFER, m_bhBuf.GetValue());
        std::array<float, 8> data{
            black_hole_start_pos_a_.x(),
            black_hole_start_pos_a_.y(),
            black_hole_start_pos_a_.z(),
            1,
            black_hole_start_pos_b_.x(),
            black_hole_start_pos_b_.y(),
            black_hole_start_pos_b_.z(),
            1};
        klgl::OpenGl::BufferData(klgl::GlBufferType::Array, std::span{data}, klgl::GlUsage::DynamicDraw);

        glGenVertexArrays(1, &m_bhVao);
        glBindVertexArray(m_bhVao);

        glBindBuffer(GL_ARRAY_BUFFER, m_bhBuf.GetValue());
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
        HandleInput();

        const float delta_t = GetLastFrameDurationSeconds() * 0.01f;
        m_angle += m_speed * delta_t;
        if (m_angle > 360.0f)
        {
            m_angle -= 360.0f;
        }

        const auto black_hole_rotation = edt::Math::RotationMatrix3dY(edt::Math::DegToRad(m_angle));
        const auto black_hole_pos_a = edt::Math::TransformPos(black_hole_rotation, black_hole_start_pos_a_);
        const auto black_hole_pos_b = edt::Math::TransformPos(black_hole_rotation, black_hole_start_pos_b_);

        {
            compute_shader_->Use();
            compute_shader_->SetUniform(bhu1, black_hole_pos_a);
            compute_shader_->SetUniform(bhu2, black_hole_pos_b);
            compute_shader_->SetUniform(u_delta_t_, delta_t);
            compute_shader_->SendUniforms();
            glDispatchCompute(total_particles_, 1, 1);
            glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
        }

        {
            klgl::OpenGl::EnableBlending();
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            just_color_->Use();
            just_color_->SetUniform(u_model_, edt::Mat4f::Identity());
            just_color_->SetUniform(u_view_, camera_.GetViewMatrix());
            just_color_->SetUniform(u_projection_, camera_.GetProjectionMatrix(GetWindow().GetAspect()));
            just_color_->SetUniform(u_color_, edt::Vec4f{1, 1, 1, 0.1f});
            // just_color_->SetUniform(u_color_, edt::Math::GetRainbowColorsA(GetTimeSeconds()).Cast<float>() / 255.f);
            just_color_->SendUniforms();

            // Draw the particles
            glPointSize(2.0f);
            glBindVertexArray(m_vao);
            glDrawArrays(GL_POINTS, 0, total_particles_);

            // Draw black holes
            just_color_->SetUniform(u_color_, edt::Vec4f{1, 0, 0, 1});
            just_color_->SendUniforms();
            glPointSize(15.0f);
            std::array<float, 8> data{
                black_hole_pos_a.x(),
                black_hole_pos_a.y(),
                black_hole_pos_a.z(),
                1.0f,
                black_hole_pos_b.x(),
                black_hole_pos_b.y(),
                black_hole_pos_b.z(),
                1.0f};
            klgl::OpenGl::BindBuffer(klgl::GlBufferType::Array, m_bhBuf);
            klgl::OpenGl::BufferSubData(klgl::GlBufferType::Array, 0, std::span{data});
            glBindVertexArray(m_bhVao);
            klgl::OpenGl::DrawArrays(klgl::GlPrimitiveType::Points, 0, 2);
        }

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

    void RenderGUI()
    {
        if (ImGui::Begin("Settings"))
        {
            camera_.Widget();
            ImGui::Separator();
            klgl::SimpleTypeWidget("Camera speed", camera_speed_);
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
                edt::Vec3f delta = static_cast<float>(forward) * camera_.GetForwardAxis();
                delta += static_cast<float>(right) * camera_.GetRightAxis();
                delta += static_cast<float>(up) * camera_.GetUpAxis();
                camera_.SetEye(camera_.GetEye() + delta * camera_speed_ * GetLastFrameDurationSeconds());
            }
        }
    }

    static constexpr edt::Vec3<size_t> particles_per_axis_{100, 100, 100};
    static constexpr size_t total_particles_ =
        particles_per_axis_.x() * particles_per_axis_.y() * particles_per_axis_.z();

    float m_speed = 700;
    float m_angle = 0;

    GLuint m_vao = 0;
    klgl::GlBufferId m_posBuf;
    klgl::GlBufferId m_velBuf;

    GLuint m_bhVao = 0;
    klgl::GlBufferId m_bhBuf;

    edt::Vec3f black_hole_start_pos_a_{5, 0, 0};
    edt::Vec3f black_hole_start_pos_b_{-5, 0, 0};
    std::shared_ptr<klgl::Shader> compute_shader_;
    std::shared_ptr<klgl::Shader> just_color_;

    float camera_speed_ = 5.f;
    klgl::UniformHandle bhu1 = klgl::UniformHandle("BlackHolePos1");
    klgl::UniformHandle bhu2 = klgl::UniformHandle("BlackHolePos2");
    klgl::UniformHandle u_delta_t_ = klgl::UniformHandle("u_delta_t");
    klgl::UniformHandle u_color_ = klgl::UniformHandle("u_color");
    klgl::UniformHandle u_model_ = klgl::UniformHandle("u_model");
    klgl::UniformHandle u_view_ = klgl::UniformHandle("u_view");
    klgl::UniformHandle u_projection_ = klgl::UniformHandle("u_projection");
    klgl::Camera3d camera_{edt::Vec3f{0, 15, 0}, {.yaw = -90, .pitch = 0}};

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
