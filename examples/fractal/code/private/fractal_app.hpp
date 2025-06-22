#pragma once

#include <klgl/events/event_listener_interface.hpp>

#include "fractal_settings.hpp"
#include "klgl/application.hpp"

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
};
