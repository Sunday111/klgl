#include "fractal_settings.hpp"

#include <random>

void FractalSettings::RandomizeColors()
{
    std::mt19937 rnd(static_cast<unsigned>(color_seed));
    std::uniform_real_distribution<float> color_distr(0, 1.0f);

    for (auto& color : colors)
    {
        color = color.Transform([&](float) { return color_distr(rnd); });
    }
}

edt::Vec2f FractalSettings::MakeJuliaConstant() const
{
    return edt::Vec2f{a * std::cos(time * c), b * std::sin(time * d)};
}
