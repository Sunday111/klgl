#include "fractal_app.hpp"

#include <klgl/template/on_scope_leave.hpp>
#include <random>

#include "imgui.h"
#include "klgl/events/event_listener_method.hpp"
#include "klgl/events/event_manager.hpp"
#include "klgl/events/mouse_events.hpp"
#include "klgl/opengl/gl_api.hpp"
#include "klgl/window.hpp"
#include "renderers/simple_gpu_renderer.hpp"

FractalApp::FractalApp() = default;
FractalApp::~FractalApp() noexcept = default;

void FractalApp::Initialize()
{
    klgl::Application::Initialize();
    event_listener_ = klgl::events::EventListenerMethodCallbacks<&FractalApp::OnMouseScroll>::CreatePtr(this);
    GetEventManager().AddEventListener(*event_listener_);

    klgl::OpenGl::SetClearColor({});
    GetWindow().SetSize(1000, 1000);
    GetWindow().SetTitle("Fractal");
    SetTargetFramerate(30.f);

    renderer_ = std::make_unique<SimpleGpuRenderer>();

    // Load shader

    settings_.RandomizeColors();
    settings_.color_positions.resize(settings_.colors.size(), 0.f);

    float delta = 1.f / static_cast<float>(settings_.color_positions.size() - 1);
    for (size_t i = 1; i != settings_.color_positions.size() - 1; ++i)
    {
        settings_.color_positions[i] = static_cast<float>(i) * delta;
    }

    settings_.color_positions.back() = 1.f;
}

void FractalApp::HandleInput()
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
            settings_.camera.eye =
                settings_.camera.eye + delta * move_speed_ * GetLastFrameDurationSeconds() / settings_.camera.zoom;
        }
    }
}

void FractalApp::Tick()
{
    klgl::Application::Tick();
    renderer_->ApplySettings(*this);

    HandleInput();
    settings_.viewport.MatchWindowSize(GetWindow().GetSize2f());

    renderer_->Render(*this);

    DrawGUI();
}

void FractalApp::OnMouseScroll(const klgl::events::OnMouseScroll& event)
{
    if (ImGui::GetIO().WantCaptureMouse) return;

    zoom_power_ += event.value.y();
    settings_.camera.zoom = std::max(std::pow(1.1f, zoom_power_), 0.1f);
}

void FractalApp::DrawGUI()
{
    if (ImGui::Begin("Settings"))
    {
        auto& c = settings_.changed;
        if (ImGui::CollapsingHeader("Julia constant"))
        {
            ImGui::SliderFloat("a", &settings_.a, 0.00001f, 1.f);
            ImGui::SliderFloat("b", &settings_.b, 0.00001f, 1.f);
            ImGui::SliderFloat("c", &settings_.c, 0.00001f, 1.f);
            ImGui::SliderFloat("d", &settings_.d, 0.00001f, 1.f);
            ImGui::Checkbox("use current time", &settings_.use_current_time);
        }

        c |= ImGui::Checkbox("Inside out space", &settings_.inside_out_space);
        c |= ImGui::SliderInt("Color Mode", &settings_.color_mode, 0, 2);

        if (ImGui::CollapsingHeader("Colors"))
        {
            for (size_t color_index = 0; color_index != settings_.colors.size(); ++color_index)
            {
                constexpr int color_edit_flags =
                    ImGuiColorEditFlags_DefaultOptions_ | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel;
                auto& color = settings_.colors[color_index];
                ImGui::PushID(&color);
                auto pop_on_exit = klgl::OnScopeLeave(&ImGui::PopID);
                c |= ImGui::ColorEdit3("Color", color.data(), color_edit_flags);

                if (color_index != 0 && color_index != settings_.colors.size() - 1)
                {
                    ImGui::SameLine();
                    c |= ImGui::SliderFloat("Pos", &settings_.color_positions[color_index], 0.0f, 1.f);
                }
            }

            c |= ImGui::Checkbox("Interpolate colors", &settings_.interpolate_colors);

            bool randomize = false;

            if (ImGui::InputInt("Color Seed:", &settings_.color_seed))
            {
                randomize = true;
            }

            ImGui::SameLine();
            if (ImGui::Button("Randomize"))
            {
                randomize = true;
                settings_.color_seed = std::bit_cast<int>(std::random_device()());
            }

            if (randomize)
            {
                settings_.RandomizeColors();
                c = true;
            }
        }
    }
    ImGui::End();
}
