#include <imgui.h>

#include <EverydayTools/Math/Math.hpp>
#include <atomic>
#include <klgl/events/event_listener_interface.hpp>
#include <klgl/events/event_listener_method.hpp>
#include <klgl/events/event_manager.hpp>
#include <klgl/events/mouse_events.hpp>
#include <klgl/mesh/mesh_data.hpp>
#include <klgl/opengl/vertex_attribute_helper.hpp>
#include <klgl/shader/shader.hpp>
#include <klgl/template/register_attribute.hpp>
#include <random>
#include <thread>

#include "fractal_settings.hpp"
#include "klgl/application.hpp"
#include "klgl/camera/camera_2d.hpp"
#include "klgl/error_handling.hpp"
#include "klgl/opengl/gl_api.hpp"
#include "klgl/reflection/matrix_reflect.hpp"  // IWYU pragma: keep
#include "klgl/rendering/curve_renderer_2d.hpp"
#include "klgl/window.hpp"

namespace klgl::curve_example
{

[[nodiscard]] static edt::Vec2f ComplexMult(edt::Vec2f a, edt::Vec2f b)
{
    return {a.x() * b.x() - a.y() * b.y(), a.x() * b.y() + a.y() * b.x()};
}

using namespace edt::lazy_matrix_aliases;  // NOLINT

class CurveFractalApp : public Application
{
    std::tuple<int, int> GetOpenGLVersion() const override { return {4, 3}; }
    static constexpr size_t kMaxCurves = 100'000;

    void Initialize() override
    {
        Application::Initialize();

        event_listener_ = klgl::events::EventListenerMethodCallbacks<&CurveFractalApp::OnMouseScroll>::CreatePtr(this);
        GetEventManager().AddEventListener(*event_listener_);

        OpenGl::SetClearColor({});
        GetWindow().SetSize(1000, 1000);
        GetWindow().SetTitle("Curve Fractal");

        OpenGl::EnableBlending();
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        constexpr Vec2f eye{0, 0};
        constexpr float sample_extent = 1.f;
        constexpr edt::FloatRange2Df world_range =
            edt::FloatRange2Df::FromMinMax(eye - sample_extent, eye + sample_extent);

        static constexpr Vec2<size_t> kNumSamples{800000, 800000};
        produced_curves_points_.resize(kMaxCurves);

        auto thread_tile = world_range.Extent() / 2.f;
        for (size_t x = 0; x != 4; ++x)
        {
            for (size_t y = 0; y != 4; ++y)
            {
                auto tile_min = Vec2<size_t>{x, y}.Cast<float>() * thread_tile + world_range.Min();
                auto thread_range = edt::FloatRange2Df::FromMinMax(tile_min, tile_min + thread_tile);
                producer_thread_.emplace_back(&CurveFractalApp::ProducerThread, this, thread_range, kNumSamples / 4);
            }
        }
    }

    void ProducerThread(const edt::FloatRange2Df world_range, const edt::Vec2<size_t> num_samples)
    {
        constexpr size_t max_iterations = 200;
        const auto world_step = 2 * world_range.Extent() / num_samples.Cast<float>();

        FractalSettings settings{10};
        settings.camera = camera;
        settings.viewport = Viewport::FromWindowSize(GetWindow().GetSize());
        settings.color_seed = std::bit_cast<int>(std::random_device()());
        settings.RandomizeColors();
        settings.DistributePositionsUniformly();
        settings.inside_out_space = true;
        settings.complex_power = 8;
        auto julia_constant = settings.MakeJuliaConstant();

        std::vector<CurveRenderer2d::ControlPoint> points;
        std::vector<edt::Vec3f> pallette;

        for (size_t iy = 0; iy != num_samples.y(); ++iy)
        {
            for (size_t ix = 0; ix != num_samples.x(); ++ix)
            {
                auto c = julia_constant;
                auto world = world_range.Min() + Vec2<size_t>{ix, iy}.Cast<float>() * world_step;

                auto z = world;

                points.clear();
                points.push_back({
                    .position = z,
                });

                size_t i = 0;
                while (i != max_iterations)
                {
                    edt::Vec2f p = z;

                    for (int j = 1; j < settings.complex_power; ++j)
                    {
                        p = ComplexMult(p, z);
                    }

                    p += c;
                    points.push_back({
                        .position = edt::Vec2f{z.y(), -z.x()},
                    });

                    if (p.SquaredLength() > 4) break;
                    z = p;
                    ++i;
                }

                if (size_t num_points = points.size(); num_points > 25)
                {
                    pallette.resize(num_points);
                    settings.ComputeColors(
                        pallette.size(),
                        [&](size_t index, const edt::Vec3f& color) { pallette[index] = color; });

                    for (size_t pi = 0; pi != points.size(); ++pi)
                    {
                        auto& point = points[pi];
                        point.color = Vec4f(pallette[pi], 1);
                        point.color.w() = (static_cast<float>(pi) / static_cast<float>(num_points)) * 0.2f;
                    }

                    {
                        std::scoped_lock lock{mutex_};
                        uint32_t num = num_produced_;
                        if (num == kMaxCurves) return;
                        uint32_t idx = num_produced_.fetch_add(1);
                        produced_curves_points_[idx] = std::move(points);
                    }
                }
            }
        }
    }

    void Tick() override
    {
        {
            const uint32_t num_produced = num_produced_;
            while (curves_.size() < num_produced)
            {
                size_t idx = curves_.size();
                auto& curve = curves_.emplace_back(std::make_unique<klgl::CurveRenderer2d>());
                curve->thickness_ = 1.f;
                auto& points = produced_curves_points_[idx];
                curve->SetPoints(points);
                points.clear();
                points.shrink_to_fit();
            }
        }

        auto viewport = Viewport::FromWindowSize(GetWindow().GetSize());
        render_transforms.Update(camera, viewport, AspectRatioPolicy::ShrinkToFit);
        // Relaxed memory order as we do not care much if something will be rendered the next frame
        for (auto& curve : curves_)
        {
            curve->Draw(viewport.size.Cast<float>(), render_transforms.world_to_view);
        }

        HandleInput();
    }

    void HandleInput()
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
                camera.eye = camera.eye + delta * move_speed_ * GetLastFrameDurationSeconds() / camera.zoom;
            }
        }
    }

    void OnMouseScroll(const klgl::events::OnMouseScroll& event)
    {
        if (ImGui::GetIO().WantCaptureMouse) return;

        zoom_power_ += event.value.y();
        camera.zoom = std::max(std::pow(1.1f, zoom_power_), 0.1f);
    }

    std::unique_ptr<klgl::events::IEventListener> event_listener_;
    Camera2d camera{};
    RenderTransforms2d render_transforms{};

    // float line_width_ = 0.001f;
    float move_speed_ = 0.5f;
    float zoom_power_ = 0.f;

    std::vector<std::jthread> producer_thread_;

    std::vector<std::vector<CurveRenderer2d::ControlPoint>> produced_curves_points_;
    std::atomic<uint32_t> num_produced_;
    std::mutex mutex_;

    std::vector<std::unique_ptr<CurveRenderer2d>> curves_;
};

void Main()
{
    CurveFractalApp app;
    app.Run();
}
}  // namespace klgl::curve_example

int main()
{
    klgl::ErrorHandling::InvokeAndCatchAll(klgl::curve_example::Main);
    return 0;
}
