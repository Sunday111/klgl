#include "klgl/texture/procedural_texture_generator.hpp"

#include <EverydayTools/Math/FloatRange.hpp>
#include <EverydayTools/Math/Math.hpp>

namespace klgl
{
struct ProceduralTextureGenerator::Helper
{
    template <typename Callback>
    static std::vector<edt::Vec4<uint8_t>> GenerateImage(
        const edt::Vec2<size_t>& size,
        const edt::FloatRange2Df& coord_range,
        size_t upscale_factor,
        Callback&& callback)
    {
        const auto sizef = size.Cast<float>();
        const auto transform = edt::Math::TranslationMatrix(coord_range.Min())
                                   .MatMul(edt::Math::ScaleMatrix(coord_range.Extent() / sizef));

        std::vector<edt::Vec4<uint8_t>> pixels;
        pixels.reserve(size.x() * size.y());

        const auto samples_per_pixel = static_cast<float>(edt::Math::Sqr(upscale_factor));
        const auto pixel_samples_per_axis = Vec2<size_t>{} + upscale_factor;
        const auto sample_offset = 1.f / (2.f * sizef * pixel_samples_per_axis.Cast<float>());

        for (const Vec2f pixel_index : PixelIndicesF(size))
        {
            const auto pixel_position = edt::Math::TransformPos(transform, pixel_index);
            Vec4f acc_color{};
            for (const Vec2f sample_idx : PixelIndicesF(pixel_samples_per_axis))
            {
                const auto sample_position = pixel_position + sample_offset * sample_idx;
                acc_color += callback(sample_position);
            }

            pixels.push_back((acc_color / samples_per_pixel).Cast<uint8_t>());
        }

        return pixels;
    }
};

std::vector<edt::Vec4<uint8_t>> ProceduralTextureGenerator::CircleMask(
    const edt::Vec2<size_t>& size,
    size_t upscale_factor)
{
    return Helper::GenerateImage(
        size,
        edt::FloatRange2Df::FromMinMax(Vec2f{} - 1, Vec2f{} + 1),
        upscale_factor,
        [&](const Vec2f& sample_position)
        {
            const uint8_t opacity = (sample_position.SquaredLength() < 1) ? 255 : 0;
            return Vec4<uint8_t>{255, 255, 255, opacity}.Cast<float>();
        });
}
}  // namespace klgl
