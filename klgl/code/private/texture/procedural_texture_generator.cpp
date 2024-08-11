#include "klgl/texture/procedural_texture_generator.hpp"

#include <EverydayTools/Math/Math.hpp>

namespace klgl
{
std::vector<edt::Vec4<uint8_t>> ProceduralTextureGenerator::CircleMask(const edt::Vec2<size_t>& size)
{
    const auto sizef = size.Cast<float>();
    const auto transform = edt::Math::TranslationMatrix(Vec2f{} - 1).MatMul(edt::Math::ScaleMatrix(2 / sizef));

    std::vector<edt::Vec4<uint8_t>> pixels;
    pixels.reserve(size.x() * size.y());

    for (Vec2f p : PixelIndicesF(size))
    {
        p = edt::Math::TransformPos(transform, p);
        const uint8_t opacity = (p.SquaredLength() < 1) ? 255 : 0;
        pixels.push_back({
            255,
            255,
            255,
            opacity,
        });
    }

    return pixels;
}
}  // namespace klgl
