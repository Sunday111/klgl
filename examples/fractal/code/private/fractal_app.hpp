#pragma once

#include <klgl/events/event_listener_interface.hpp>

#include "fractal_settings.hpp"
#include "klgl/application.hpp"
#include "klgl/ui/imgui_value_combo.hpp"

namespace klgl::events
{
class OnMouseScroll;
}

class FractalRenderer;
class InterpolationWidget;

class FractalApp : public klgl::Application
{
public:
    FractalApp();
    ~FractalApp() noexcept override;

    std::tuple<int, int> GetOpenGLVersion() const override { return {4, 5}; }

    static constexpr size_t kMaxIterations = 200;

    void Initialize() override;

    void HandleInput();

    void Tick() override;

    void OnMouseScroll(const klgl::events::OnMouseScroll& event);
    std::vector<edt::Vec4u8> CaptureScreenshot() const;

    std::unique_ptr<FractalRenderer> renderer_;
    std::unique_ptr<InterpolationWidget> interpolation_widget_;

    std::unique_ptr<klgl::events::IEventListener> event_listener_;
    float zoom_power_ = 0.f;
    float move_speed_ = 0.5f;
    FractalSettings settings_{10};
    bool screenshot = false;
    bool screenshot_with_ui = false;

    template <typename T>
    [[nodiscard]] static std::unique_ptr<FractalRenderer> RendererFactoryFn(size_t num_iterations)
    {
        return std::make_unique<T>(num_iterations);
    }

    using RendererFactory = std::unique_ptr<FractalRenderer> (*)(size_t iterations);
    klgl::ImGuiValueCombo<RendererFactory> renderer_combo_{"Renderer"};
};
