#include <imgui.h>

#include <EverydayTools/Math/Math.hpp>

#include "klgl/application.hpp"
#include "klgl/error_handling.hpp"
#include "klgl/opengl/gl_api.hpp"
#include "klgl/rendering/painter2d.hpp"
#include "klgl/ui/simple_type_widget.hpp"
#include "klgl/window.hpp"

namespace klgl::painter2d_example
{

class Painter2dApp : public Application
{
    void Initialize() override
    {
        Application::Initialize();
        OpenGl::SetClearColor({});
        GetWindow().SetSize(1000, 1000);
        GetWindow().SetTitle("Painter 2d");
        painter_ = std::make_unique<Painter2d>();
    }

    void Tick() override
    {
        constexpr edt::Vec4u8 red{255, 0, 0, 255};
        constexpr edt::Vec4u8 green{0, 255, 0, 255};
        // constexpr edt::Vec4u8 blue{0, 0, 255, 255};
        constexpr edt::Vec4u8 white{255, 255, 255, 255};

        painter_->BeginDraw();
        painter_->FillRect({.center = {}, .size = {1, 1}, .color = red});
        painter_->RectLines({.center = {}, .size = {1, 1}, .color = white}, {0.01f});
        painter_->FillCircle({.center = {-0.3f, 0.3f}, .size = {.2f, .2f}, .color = green});
        painter_->FillCircle({.center = {0.3f, 0.3f}, .size = {.2f, .2f}, .color = green});
        painter_->FillCircle({.center = {0, -0.25f}, .size = {.8f, .2f}, .color = green});
        painter_->FillTriangle({.a = {-0.5, 0.5}, .b = {-0.3f, 0.6f}, .c = {-0.2f, 0.5}, .color = red});
        painter_->FillTriangle({.a = {0.5, 0.5}, .b = {0.3f, 0.6f}, .c = {0.2f, 0.5}, .color = red});
        // painter_->FillTriangle({.a = a, .b = b, .c = c, .color = blue});
        painter_->TriangleLines({.a = a, .b = b, .c = c, .color = white}, triangle_line_width_);
        painter_->DrawLine({.a = line_a_, .b = line_b_, .color = white, .width = line_width_});

        painter_->EndDraw();

        if (ImGui::CollapsingHeader("Triangle"))
        {
            SimpleTypeWidget("a", a);
            SimpleTypeWidget("b", b);
            SimpleTypeWidget("c", c);
            ImGui::SliderFloat("inner line width", &triangle_line_width_.inner, 0.f, 0.5f);
            ImGui::SliderFloat("outer line width", &triangle_line_width_.outer, 0.f, 0.5f);
        }

        if (ImGui::CollapsingHeader("Line"))
        {
            SimpleTypeWidget("a", line_a_);
            SimpleTypeWidget("b", line_b_);
            SimpleTypeWidget("w", line_width_);
        }
    }

    std::unique_ptr<Painter2d> painter_;
    edt::Vec2f a{-1, -1}, b{0.2f, -1}, c{-1, 1};
    Painter2d::LineWidth triangle_line_width_{.inner = 0.01f};

    edt::Vec2f line_a_{-1, 0};
    edt::Vec2f line_b_{1, 0};
    float line_width_ = 0.05f;
};

void Main()
{
    Painter2dApp app;
    app.Run();
}
}  // namespace klgl::painter2d_example

int main()
{
    klgl::ErrorHandling::InvokeAndCatchAll(klgl::painter2d_example::Main);
    return 0;
}
