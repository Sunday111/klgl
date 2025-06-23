#include "fractal_app.hpp"

#include <klgl/template/on_scope_leave.hpp>

#include "clipboard.hpp"
#include "imgui.h"
#include "interpolation_widget.hpp"
#include "klgl/events/event_listener_method.hpp"
#include "klgl/events/event_manager.hpp"
#include "klgl/events/mouse_events.hpp"
#include "klgl/opengl/gl_api.hpp"
#include "klgl/window.hpp"
#include "renderers/counting_renderer.hpp"
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

    renderer_ = std::make_unique<SimpleGpuRenderer>(kMaxIterations);
    interpolation_widget_ = std::make_unique<InterpolationWidget>(kMaxIterations + 1);

    settings_.RandomizeColors();
    settings_.DistributePositionsUniformly();
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

std::vector<edt::Vec4u8> FractalApp::CaptureScreenshot() const
{
    std::vector<edt::Vec4u8> pixels;
    auto size = GetWindow().GetSize();
    pixels.resize(size.x() * size.y());
    klgl::OpenGl::ReadPixels(0, 0, size.x(), size.y(), GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
    return pixels;
}

void FractalApp::Tick()
{
    klgl::Application::Tick();

    HandleInput();
    settings_.SetCurrentTime(GetTimeSeconds());

    settings_.SetViewport(klgl::Viewport{
        .position = {},
        .size = GetWindow().GetSize(),
    });
    if (settings_.changed)
    {
        renderer_->ApplySettings(settings_);
        settings_.changed = false;
    }
    renderer_->Render(settings_);

    if (screenshot)
    {
        glReadBuffer(screenshot_with_ui ? GL_FRONT : GL_BACK);
        auto resolution = GetWindow().GetSize();
        auto pixels = CaptureScreenshot();
        Clipboard::AddImage(resolution.Cast<size_t>(), pixels);
        screenshot = false;
    }

    klgl::Viewport widget_viewport;
    widget_viewport.MatchWindowSize(GetWindow().GetSize());
    widget_viewport.size.y() = 50;
    interpolation_widget_->Render(widget_viewport, settings_);

    if (ImGui::Begin("Settings"))
    {
        if (ImGui::SliderInt("Renderer Kind", &renderer_kind_, 0, 1))
        {
            settings_.changed = true;
            switch (renderer_kind_)
            {
            case 0:
                renderer_ = std::make_unique<SimpleGpuRenderer>(kMaxIterations);
                break;
            case 1:
                renderer_ = std::make_unique<CountingRenderer>(kMaxIterations);
                break;
            }
        }
        screenshot = ImGui::Button("Screenshot to clipboard");
        ImGui::SameLine();
        ImGui::Checkbox("With interface", &screenshot_with_ui);

        settings_.DrawGUI();
    }
    ImGui::End();
}

void FractalApp::OnMouseScroll(const klgl::events::OnMouseScroll& event)
{
    if (ImGui::GetIO().WantCaptureMouse) return;

    zoom_power_ += event.value.y();
    settings_.camera.zoom = std::max(std::pow(1.1f, zoom_power_), 0.1f);
}
