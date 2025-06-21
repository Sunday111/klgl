#include <fmt/format.h>
#include <imgui.h>

#include <EverydayTools/Math/Math.hpp>
#include <klgl/events/event_listener_method.hpp>
#include <klgl/events/event_manager.hpp>
#include <klgl/events/mouse_events.hpp>
#include <klgl/template/on_scope_leave.hpp>
#include <klgl/ui/simple_type_widget.hpp>

#include "fractal_app.hpp"
#include "klgl/error_handling.hpp"
#include "klgl/reflection/matrix_reflect.hpp"  // IWYU pragma: keep

void Main()
{
    FractalApp app;
    app.Run();
}

int main()
{
    klgl::ErrorHandling::InvokeAndCatchAll(Main);
    return 0;
}
