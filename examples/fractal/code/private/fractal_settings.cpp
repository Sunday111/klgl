#include "fractal_settings.hpp"

#include <bit>
#include <klgl/error_handling.hpp>
#include <klgl/template/on_scope_leave.hpp>
#include <random>

#include "imgui.h"

FractalSettings::FractalSettings(size_t num_colors_) : num_colors(num_colors_)
{
    klgl::ErrorHandling::Ensure(num_colors > 1, "Need at least two colors");
    color_positions.resize(num_colors, 0);
    colors.resize(num_colors, edt::Vec3f{});
}

void FractalSettings::RandomizeColors()
{
    std::mt19937 rnd(static_cast<unsigned>(color_seed));
    std::uniform_real_distribution<float> color_distr(0, 1.0f);

    for (auto& color : colors)
    {
        color = color.Transform([&](float) { return color_distr(rnd); });
    }

    changed = true;
}

void FractalSettings::DistributePositionsUniformly()
{
    float delta = 1.f / static_cast<float>(num_colors - 1);
    for (size_t i = 1; i != num_colors - 1; ++i)
    {
        color_positions[i] = static_cast<float>(i) * delta;
    }

    color_positions.back() = 1.f;
    changed = true;
}

edt::Vec2f FractalSettings::MakeJuliaConstant() const
{
    return edt::Vec2f{a * std::cos(time * c), b * std::sin(time * d)};
}

[[nodiscard]] constexpr edt::Vec3f rgb2hsv(edt::Vec3f in)
{
    edt::Vec3f out;

    float min = in[0] < in[1] ? in[0] : in[1];
    min = min < in[2] ? min : in[2];

    float max = in[0] > in[1] ? in[0] : in[1];
    max = max > in[2] ? max : in[2];

    out[2] = max;  // v
    float delta = max - min;
    if (delta < 0.00001)
    {
        out[1] = 0;
        out[0] = 0;  // undefined, maybe nan?
        return out;
    }
    if (max > 0.0)
    {                            // NOTE: if Max is == 0, this divide would cause a crash
        out[1] = (delta / max);  // s
    }
    else
    {
        // if max is 0, then r = g = b = 0
        // s = 0, h is undefined
        out[1] = 0.0;
        out[0] = NAN;  // its now undefined
        return out;
    }

    if (in[0] >= max)  // > is bogus, just keeps compilor happy
    {
        out[0] = (in[1] - in[2]) / delta;  // between yellow & magenta
    }
    else if (in[1] >= max)
    {
        out[0] = 2 + (in[2] - in[0]) / delta;  // between cyan & yellow
    }
    else
    {
        out[0] = 4 + (in[0] - in[1]) / delta;  // between magenta & cyan
    }

    out[0] *= 60.0;  // degrees

    if (out[0] < 0.0) out[0] += 360.0;

    return out;
}

[[nodiscard]] constexpr edt::Vec3f hsv2rgb(edt::Vec3f in)
{
    edt::Vec3f out;

    if (in[1] <= 0.0)
    {  // < is bogus, just shuts up warnings
        out[0] = in[2];
        out[1] = in[2];
        out[2] = in[2];
        return out;
    }
    float hh = in[0];
    if (hh >= 360.0) hh = 0.0;
    hh /= 60.0;
    auto i = static_cast<int64_t>(hh);
    float ff = hh - static_cast<float>(i);
    float p = in[2] * (1 - in[1]);
    float q = in[2] * (1 - (in[1] * ff));
    float t = in[2] * (1 - (in[1] * (1 - ff)));

    switch (i)
    {
    case 0:
        out[0] = in[2];
        out[1] = t;
        out[2] = p;
        break;
    case 1:
        out[0] = q;
        out[1] = in[2];
        out[2] = p;
        break;
    case 2:
        out[0] = p;
        out[1] = in[2];
        out[2] = t;
        break;

    case 3:
        out[0] = p;
        out[1] = q;
        out[2] = in[2];
        break;
    case 4:
        out[0] = t;
        out[1] = p;
        out[2] = in[2];
        break;
    case 5:
    default:
        out[0] = in[2];
        out[1] = p;
        out[2] = q;
        break;
    }
    return out;
}

edt::Vec3f LerpHSV(edt::Vec3f a, edt::Vec3f b, float t)
{
    float x = a[0] / 360, y = b[0] / 360;
    float delta = std::fmod(y - x + 1, 1.0f);
    if (delta > 0.5f) delta -= 1.0f;
    float h = 360.f * std::fmod(x + t * delta, 1.0f);

    return edt::Vec3f{h, std::lerp(a[1], b[1], t), std::lerp(a[2], b[2], t)};
}

edt::Vec3f FractalSettings::LerpColors(edt::Vec3f from, edt::Vec3f to, float t) const
{
    if (interpolate_with_hsv)
    {
        return hsv2rgb(LerpHSV(rgb2hsv(from), rgb2hsv(to), t));
    }

    return edt::Math::Lerp(from, to, t);
}

void FractalSettings::DrawGUI()
{
    if (ImGui::CollapsingHeader("Julia constant"))
    {
        ImGui::SliderFloat("a", &a, 0.00001f, 1.f);
        ImGui::SliderFloat("b", &b, 0.00001f, 1.f);
        ImGui::SliderFloat("c", &c, 0.00001f, 1.f);
        ImGui::SliderFloat("d", &d, 0.00001f, 1.f);
        ImGui::Checkbox("use current time", &use_current_time);
    }

    changed |= ImGui::Checkbox("Inside out space", &inside_out_space);
    changed |= ImGui::SliderInt("Color Mode", &color_mode, 0, 3);

    if (ImGui::CollapsingHeader("Colors"))
    {
        for (size_t color_index = 0; color_index != colors.size(); ++color_index)
        {
            constexpr int color_edit_flags =
                ImGuiColorEditFlags_DefaultOptions_ | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel;
            auto& color = colors[color_index];
            ImGui::PushID(&color);
            auto pop_on_exit = klgl::OnScopeLeave(&ImGui::PopID);
            changed |= ImGui::ColorEdit3("Color", color.data(), color_edit_flags);

            if (color_index != 0 && color_index != colors.size() - 1)
            {
                ImGui::SameLine();
                changed |= ImGui::SliderFloat("Pos", &color_positions[color_index], 0.0f, 1.f);
            }
        }

        changed |= ImGui::Checkbox("Interpolate colors", &interpolate_colors);
        if (interpolate_colors)
        {
            changed |= ImGui::Checkbox("Interpolate in HSV space", &interpolate_with_hsv);
        }

        bool randomize = false;

        if (ImGui::InputInt("Color Seed:", &color_seed))
        {
            randomize = true;
        }

        ImGui::SameLine();
        if (ImGui::Button("Randomize"))
        {
            randomize = true;
            color_seed = std::bit_cast<int>(std::random_device()());
        }

        if (randomize)
        {
            RandomizeColors();
            changed = true;
        }
    }
}
