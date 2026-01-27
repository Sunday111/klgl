#include <imgui.h>

#include <EverydayTools/Math/Math.hpp>
#include <klgl/camera/camera_2d.hpp>
#include <klgl/rendering/curve_renderer_2d.hpp>

#include "klgl/application.hpp"
#include "klgl/error_handling.hpp"
#include "klgl/opengl/gl_api.hpp"
#include "klgl/rendering/painter2d.hpp"
#include "klgl/window.hpp"

namespace klgl::painter2d_example
{

/*
 * Performs one RK4 step
 *
 * n      = dimension of state vector
 * t      = current time
 * dt     = timestep
 * y      = state vector (updated in place)
 * derivs = function computing dy/dt
 */
template <size_t n, typename F>
void rk4_step(double t, double dt, std::span<double, n> y, F derivs)
{
    auto dt6 = dt / 6, dt2 = dt * 0.5;
    std::array<double, n> k1{}, k2{}, k3{}, k4{}, y_temp{};

    // k1
    derivs(t, y, k1);

    // k2
    for (size_t i = 0; i != n; i++) y_temp[i] = y[i] + dt2 * k1[i];
    derivs(t + dt2, y_temp, k2);

    // k3
    for (size_t i = 0; i != n; i++) y_temp[i] = y[i] + dt2 * k2[i];
    derivs(t + dt2, y_temp, k3);

    // k4
    for (size_t i = 0; i != n; i++) y_temp[i] = y[i] + dt * k3[i];
    derivs(t + dt, y_temp, k4);

    // combine
    for (size_t i = 0; i != n; i++) y[i] += dt6 * (k1[i] + 2.0 * k2[i] + 2.0 * k3[i] + k4[i]);
}

template <size_t n, typename F>
    requires(n % 2 == 0)  // state must be [q, v]
void symplectic_euler_step(double t, double dt, std::span<double, n> y, F derivs)
{
    constexpr size_t k = n / 2;
    std::array<double, n> dydt{};
    derivs(t, y, dydt);
    for (size_t i = 0; i < k; ++i) y[k + i] += dt * dydt[k + i];
    for (size_t i = 0; i < k; ++i) y[i] += dt * y[k + i];
}

template <size_t n, typename F>
    requires(n % 2 == 0)  // state must be [q, v]
void velocity_verlet_step(double t, double dt, std::span<double, n> y, F derivs)
{
    constexpr size_t k = n / 2;
    std::array<double, n> dydt{};

    derivs(t, y, dydt);

    // half-step velocities
    for (size_t i = 0; i < k; ++i) y[k + i] += 0.5 * dt * dydt[k + i];

    // full-step positions
    for (size_t i = 0; i < k; ++i) y[i] += dt * y[k + i];

    // a_{n+1}
    derivs(t + dt, y, dydt);

    // finish velocities
    for (size_t i = 0; i < k; ++i) y[k + i] += 0.5 * dt * dydt[k + i];
}

struct DoublePendulumParams
{
    double m1, m2;
    double l1, l2;
    double g;
};

void double_pendulum_derivs() {}

struct DoublePendulum
{
    std::tuple<Vec2f, Vec2f, Vec2f> get_points() const
    {
        auto th0 = state[0];
        auto p1 = p0 + Vec2{
                           params.l1 * std::sin(th0),
                           -params.l1 * std::cos(th0),
                       };

        auto th1 = state[1];
        auto p2 = p1 + Vec2{
                           params.l2 * std::sin(th1),
                           -params.l2 * std::cos(th1),
                       };

        return {p0.Cast<float>(), p1.Cast<float>(), p2.Cast<float>()};
    }

    void time_step(double dt)
    {
        rk4_step<4>(
            // symplectic_euler_step<4>(
            // velocity_verlet_step<4>(
            current_time,
            dt,
            state,
            [&]([[maybe_unused]] double t, std::span<const double, 4> y, std::span<double, 4> dydt)
            {
                auto th1 = y[0];
                auto th2 = y[1];
                auto w1 = y[2];
                auto w2 = y[3];
                auto dth = th1 - th2;
                auto sinth1 = std::sin(th1);
                auto sinth2 = std::sin(th2);
                auto sindth = std::sin(dth), cosdth = std::cos(dth);
                auto denom = (params.m1 + params.m2) - params.m2 * cosdth * cosdth;

                dydt[0] = w1;
                dydt[1] = w2;
                dydt[2] = (-params.g * (params.m1 + params.m2) * sinth1 + params.m2 * params.g * sinth2 * cosdth -
                           params.m2 * sindth * (params.l2 * w2 * w2 + params.l1 * w1 * w1 * cosdth)) /
                          (params.l1 * denom);
                dydt[3] = (params.g * (params.m1 + params.m2) * sinth1 * cosdth -
                           params.g * (params.m1 + params.m2) * sinth2 +
                           (params.m1 + params.m2) * sindth * (params.l1 * w1 * w1 + params.l2 * w2 * w2 * cosdth)) /
                          (params.l2 * denom);
            });
        current_time += dt;
    }

    std::array<double, 4> state = {
        std::numbers::pi_v<double> / 2.0,  // θ1
        std::numbers::pi_v<double> / 2.0,  // θ2
        0.0,                               // ω1
        0.0                                // ω2
    };
    double current_time = 0.0;
    Vec2<double> p0;
    DoublePendulumParams params = {.m1 = 1.0, .m2 = 1.0, .l1 = 0.45, .l2 = 0.45, .g = 9.81};
};

class PendulumApp : public Application
{
    void Initialize() override
    {
        Application::Initialize();
        OpenGl::SetClearColor({});
        GetWindow().SetSize(2000, 2000);
        GetWindow().SetTitle("Pendulum App");
        painter_ = std::make_unique<Painter2d>();
        curve_renderer_.Init();

        // for (int i = -2; i != 3; ++i)
        for (int i = 0; i != 1; ++i)
        {
            auto& p = pendulums.emplace_back();
            p.state[0] = std::numbers::pi_v<double>;
            p.state[1] = (static_cast<double>(i - 1) / 100000) + std::numbers::pi_v<double>;
            p.p0.x() = static_cast<double>(i) / 3;
        }

        Vec2f p = std::get<2>(pendulums[0].get_points());
        curve_points_.push_back(
            CurveRenderer2d::ControlPoint{
                .position = p,
                .color = Vec4f(edt::Math::GetRainbowColors(GetTimeSeconds()).Cast<float>() / 256, 1.f)});
    }

    void Tick() override
    {
        constexpr edt::Vec4u8 red{255, 0, 0, 255};
        constexpr edt::Vec4u8 green{0, 255, 0, 255};
        // constexpr edt::Vec4u8 blue{0, 0, 255, 255};
        // constexpr edt::Vec4u8 white{255, 255, 255, 255};

        auto& window = GetWindow();
        auto viewport = Viewport::FromWindowSize(window.GetSize());
        render_transforms_.Update(camera, viewport, AspectRatioPolicy::ShrinkToFit);
        painter_->SetViewMatrix(render_transforms_.world_to_view.Transposed());

        ImGui::SliderFloat("Time scale", &time_scale, 0.1f, 10.f);

        painter_->BeginDraw();
        double target_time = static_cast<double>(GetTimeSeconds() * time_scale);

        while (pendulums[0].current_time < target_time)
        {
            pendulums[0].time_step(0.0001);

            Vec2f p = std::get<2>(pendulums[0].get_points());
            float dist = (p - curve_points_.back().position).Length();
            if (dist > 0.03f)
            {
                total_curve_dist += dist;
                curve_points_.push_back(
                    CurveRenderer2d::ControlPoint{
                        .position = p,
                        .color = Vec4f(edt::Math::GetRainbowColors(total_curve_dist / 40.f).Cast<float>() / 256, 1.f)});
            }
        }

        for (auto& p : pendulums)
        {
            auto [p0, p1, p2] = p.get_points();
            painter_->DrawLine({.a = p0, .b = p1, .color = red, .width = 0.001f});
            painter_->DrawLine({.a = p1, .b = p2, .color = red, .width = 0.001f});
            painter_->FillCircle({.center = p0, .size = {0.05f, 0.05f}, .color = green});
            painter_->FillCircle({.center = p1, .size = {0.05f, 0.05f}, .color = green});
            painter_->FillCircle({.center = p2, .size = {0.05f, 0.05f}, .color = green});
        }
        painter_->EndDraw();

        // dirty, I know
        ssize_t max_cuve_points = 3000;
        curve_points_.erase(
            curve_points_.begin(),
            curve_points_.begin() +
                static_cast<ssize_t>(std::max<ssize_t>(std::ssize(curve_points_), max_cuve_points) - max_cuve_points));

        for (size_t i = 1; auto& p : curve_points_)
        {
            p.color.w() = static_cast<float>(i++) / static_cast<float>(curve_points_.size());
        }

        if (curve_points_.size() > 1)
        {
            curve_renderer_.SetPoints(curve_points_);
            curve_renderer_.Draw(viewport.size.Cast<float>(), render_transforms_.world_to_view);
        }
    }

    Camera2d camera;
    RenderTransforms2d render_transforms_;
    std::unique_ptr<Painter2d> painter_;
    std::vector<DoublePendulum> pendulums;
    std::vector<CurveRenderer2d::ControlPoint> curve_points_;
    CurveRenderer2d curve_renderer_;
    float time_scale = 1.f;
    float total_curve_dist = 0.0f;
};

void Main()
{
    PendulumApp app;
    app.Run();
}
}  // namespace klgl::painter2d_example

int main()
{
    klgl::ErrorHandling::InvokeAndCatchAll(klgl::painter2d_example::Main);
    return 0;
}
