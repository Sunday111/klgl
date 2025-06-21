#pragma once

#include <EverydayTools/Math/Math.hpp>
#include <EverydayTools/Math/Matrix.hpp>
#include <array>
#include <vector>

#include "klgl/camera/camera_2d.hpp"

struct FractalParams
{
    float a = 0.7785f;
    float b = 0.7785f;
    float c = 0.3f;
    float d = 0.5f;
    float time = 0.0f;
    bool use_current_time = true;
    int color_seed = 1234;
    bool interpolate_colors = true;
    std::array<edt::Vec3f, 10> colors;
    std::vector<float> color_positions;
    bool inside_out_space = true;
    bool changed = true;
    int color_mode = 0;
    klgl::Camera2d camera;
    klgl::Viewport viewport;

    void RandomizeColors();

    edt::Vec2f MakeJuliaConstant(float current_time);

    template <typename Callback>
    void ComputeColors(size_t n, Callback cb) const
    {
        std::vector<size_t> indices(colors.size());
        for (size_t i = 0; i != colors.size(); ++i) indices[i] = i;

        constexpr std::less pred{};
        auto proj = [&](size_t i) -> const float&
        {
            return color_positions[i];
        };

        std::ranges::sort(indices, pred, proj);

        const size_t last = color_positions.size() - 1;
        for (size_t i = 0; i != n; ++i)
        {
            const float fi = static_cast<float>(i) / static_cast<float>(n - 1);
            const auto li = std::ranges::lower_bound(indices, fi, pred, proj);
            const auto left = std::distance(li, indices.end()) < 2 ? last - 1 : *li;
            const auto right = left + 1;

            if (interpolate_colors)
            {
                float t = (fi - color_positions[left]) / (color_positions[right] - color_positions[left]);
                auto color = edt::Math::Lerp(colors[left], colors[right], t);
                cb(i, color);
            }
            else
            {
                bool use_left = fi - color_positions[left] < color_positions[right] - fi;
                auto color = colors[use_left ? left : right];
                cb(i, color);
            }
        }
    }
};
