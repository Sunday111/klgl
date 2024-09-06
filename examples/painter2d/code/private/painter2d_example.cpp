#include <imgui.h>

#include <EverydayTools/Math/Math.hpp>

#include "klgl/application.hpp"
#include "klgl/error_handling.hpp"
#include "klgl/opengl/gl_api.hpp"
#include "klgl/rendering/painter2d.hpp"
#include "klgl/ui/simple_type_widget.hpp"
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
        constexpr edt::Vec4u8 red{255, 0, 0, 255};
        constexpr edt::Vec4u8 green{0, 255, 0, 255};
        constexpr edt::Vec4u8 blue{0, 0, 255, 255};

        painter_->BeginDraw();
        painter_->DrawRect({.center = {}, .size = {1, 1}, .color = red});
        painter_->DrawCircle({.center = {-0.3f, 0.3f}, .size = {.2f, .2f}, .color = green});
        painter_->DrawCircle({.center = {0.3f, 0.3f}, .size = {.2f, .2f}, .color = green});
        painter_->DrawCircle({.center = {0, -0.25f}, .size = {.8f, .2f}, .color = green});
        painter_->DrawTriangle({.a = {-0.5, 0.5}, .b = {-0.3f, 0.6f}, .c = {-0.2f, 0.5}, .color = red});
        painter_->DrawTriangle({.a = {0.5, 0.5}, .b = {0.3f, 0.6f}, .c = {0.2f, 0.5}, .color = red});
        painter_->DrawTriangle({.a = a, .b = b, .c = c, .color = blue});

        painter_->EndDraw();

        if (ImGui::CollapsingHeader("Triangle"))
        {
            klgl::SimpleTypeWidget("a", a);
            klgl::SimpleTypeWidget("b", b);
            klgl::SimpleTypeWidget("c", c);
        }
    }

    std::unique_ptr<klgl::Painter2d> painter_;
    edt::Vec2f a{-1, -1}, b{0.2f, -1}, c{-1, 1};
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
