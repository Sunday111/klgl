#pragma once

#include <klgl/events/event_listener_interface.hpp>

#include "fractal_settings.hpp"
#include "klgl/application.hpp"

namespace klgl::events
{
class OnMouseScroll;
}

class FractalRenderer;

class FractalApp : public klgl::Application
{
public:
    FractalApp();
    ~FractalApp() noexcept override;

    void Initialize() override;

    void HandleInput();

    void Tick() override;

    void OnMouseScroll(const klgl::events::OnMouseScroll& event);

    void DrawGUI();

    std::unique_ptr<FractalRenderer> renderer_;

    std::unique_ptr<klgl::events::IEventListener> event_listener_;
    float zoom_power_ = 0.f;
    float move_speed_ = 0.5f;
    FractalSettings settings_;
};
