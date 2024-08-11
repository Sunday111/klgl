#pragma once

#include <vector>

#include "EverydayTools/Math/Matrix.hpp"
#include "EverydayTools/Ranges/ArrayIndices2d.hpp"

namespace klgl
{
using namespace edt::lazy_matrix_aliases;  // NOLINT

class ProceduralTextureGenerator
{
public:
    struct Helper;

    // Accepts texture size as Vec2 and yields pixel indices as Vec2
    [[nodiscard]] static constexpr auto PixelIndices(const Vec2<size_t>& texture_size)
    {
        return edt::ArrayIndices2d(texture_size.y(), texture_size.x()) |
               std::views::transform(
                   [](std::tuple<size_t, size_t> yx) {
                       return Vec2<size_t>{std::get<1>(yx), std::get<0>(yx)};
                   });
    }

    // Same as PixelIndices but yields pairs of indices as floats
    [[nodiscard]] static constexpr auto PixelIndicesF(const Vec2<size_t>& texture_size)
    {
        return PixelIndices(texture_size) | std::views::transform(&Vec2<size_t>::Cast<float>);
    }

    [[nodiscard]] static std::vector<edt::Vec4<uint8_t>> CircleMask(
        const edt::Vec2<size_t>& size,
        size_t upscale_factor = 1);
};
}  // namespace klgl
