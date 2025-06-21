#include "fractal_settings.hpp"

#include <random>


void FractalParams::RandomizeColors()
{
    std::mt19937 rnd(static_cast<unsigned>(color_seed));
    std::uniform_real_distribution<float> color_distr(0, 1.0f);

    for (auto& color : colors)
    {
        color = color.Transform([&](float) { return color_distr(rnd); });
    }
}

edt::Vec2f FractalParams::MakeJuliaConstant(float current_time)
{
    if (use_current_time) time = current_time;
    return edt::Vec2f{a * std::cos(time * c), b * std::sin(time * d)};
}
