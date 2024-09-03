#include <EverydayTools/Math/Math.hpp>

#include "klgl/application.hpp"
#include "klgl/error_handling.hpp"
#include "klgl/opengl/gl_api.hpp"
#include "klgl/rendering/painter2d.hpp"
#include "klgl/window.hpp"

class Painter2dApp : public klgl::Application
{
    void Initialize() override
    {
        klgl::Application::Initialize();
        klgl::OpenGl::SetClearColor({});
        GetWindow().SetSize(1000, 1000);
        GetWindow().SetTitle("Painter 2d");
        painter_ = std::make_unique<klgl::Painter2d>(*this);
    }

    void Tick() override
    {
        painter_->DrawRect({.center = {}, .size = {.5f, .5f}, .color = {1, 0, 0, 1}});
        painter_->DrawCircle({.center = {-0.3f, 0.3f}, .size = {.1f, .1f}, .color = {0, 1, 0, 1}});
        painter_->DrawCircle({.center = {0.3f, 0.3f}, .size = {.1f, .1f}, .color = {0, 1, 0, 1}});
        painter_->DrawCircle({.center = {0, -0.25f}, .size = {.4f, .1f}, .color = {0, 1, 0, 1}});
    }

    std::unique_ptr<klgl::Painter2d> painter_;
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
