#pragma once

#include <concepts>
#include <functional>
#include <optional>
#include <ranges>

#include "EverydayTools/Math/Matrix.hpp"
#include "ass/fixed_bitset.hpp"
#include "klgl/error_handling.hpp"

template <std::integral T>
[[nodiscard]] inline constexpr auto Make2dCoords(edt::Vec2<T> size)
{
    return std::views::join(
        std::views::iota(T{0}, size.y()) |
        std::views::transform(
            [w = size.x()](T y)
            { return std::views::iota(T{0}, w) | std::views::transform([y](T x) { return edt::Vec2<T>{x, y}; }); }));
}

[[nodiscard]] inline constexpr size_t Coord2dToIndex(edt::Vec2<size_t> coord, size_t width)
{
    return width * coord.y() + coord.x();
}

class BlockPrefab
{
public:
    [[nodiscard]] inline constexpr auto AllCoords() const
    {
        return Make2dCoords(size) | std::views::filter(std::bind_front(&BlockPrefab::GetCell, this));
    }

    [[nodiscard]] inline constexpr bool GetCell(edt::Vec2<size_t> coord) const
    {
        return bits.Get(Coord2dToIndex(coord, 4));
    }

    inline constexpr void SetCell(edt::Vec2<size_t> coord, bool value = true)
    {
        bits.Set(Coord2dToIndex(coord, 4), value);
    }

    edt::Vec2<size_t> size{0, 0};
    ass::FixedBitset<16> bits;
};

[[nodiscard]] inline constexpr BlockPrefab ParseBlockPrefab(std::string_view txt)
{
    std::optional<char> block_char;
    BlockPrefab b;
    size_t num_bits = 0;
    size_t cell_idx = 0;
    for (char c : txt)
    {
        if (c >= 'A' && c <= 'Z')
        {
            if (block_char.has_value())
            {
                klgl::ErrorHandling::Ensure(block_char.value() == c, "Multiple block characters in block definition");
            }
            else
            {
                block_char = c;
            }

            klgl::ErrorHandling::Ensure(cell_idx < 16, "Too many cells.");
            const edt::Vec2<size_t> coord{cell_idx % 4, 3 - cell_idx / 4};
            b.SetCell(coord);
            b.size.x() = std::max(b.size.x(), coord.x() + 1);
            b.size.y() = std::max(b.size.y(), coord.y() + 1);
            ++num_bits;
        }
        else if (c != '.')
        {
            continue;
        }

        ++cell_idx;
    }

    klgl::ErrorHandling::Ensure(num_bits = 4, "Expected 4 cells to be occupied.");

    return b;
}

[[nodiscard]] inline constexpr auto
MakeRotationsArray(std::string_view rot_0, std::string_view rot_90, std::string_view rot_180, std::string_view rot_270)
{
    return std::array<BlockPrefab, 4>{
        ParseBlockPrefab(rot_0),
        ParseBlockPrefab(rot_90),
        ParseBlockPrefab(rot_180),
        ParseBlockPrefab(rot_270)};
};

inline constexpr auto RotationsS = MakeRotationsArray(
    R"(
. S S .
S S . .
. . . .
. . . .
)",
    R"(
. S . .
. S S .
. . S .
. . . .
)",
    R"(
. . . .
. S S .
S S . .
. . . .
)",
    R"(
S . . .
S S . .
. S . .
. . . .
)");

inline constexpr auto RotationsI = MakeRotationsArray(
    R"(
. . . .
I I I I
. . . .
. . . .
)",
    R"(
. . I .
. . I .
. . I .
. . I .
)",
    R"(
. . . .
. . . .
I I I I
. . . .
)",
    R"(
. I . .
. I . .
. I . .
. I . .
)");

inline constexpr auto RotationsO = MakeRotationsArray(
    R"(
. O O .
. O O .
. . . .
. . . .
)",
    R"(
. O O .
. O O .
. . . .
. . . .
)",
    R"(
. O O .
. O O .
. . . .
. . . .
)",
    R"(
. O O .
. O O .
. . . .
. . . .
)");

inline constexpr auto RotationsT = MakeRotationsArray(
    R"(
. T . .
T T T .
. . . .
. . . .
)",
    R"(
. T . .
. T T .
. T . .
. . . .
)",
    R"(
. . . .
T T T .
. T . .
. . . .
)",
    R"(
. T . .
T T . .
. T . .
. . . .
)");

inline constexpr auto RotationsL = MakeRotationsArray(
    R"(
. . L .
L L L .
. . . .
. . . .
)",
    R"(
. L . .
. L . .
. L L .
. . . .
)",
    R"(
. . . .
L L L .
L . . .
. . . .
)",
    R"(
L L . .
. L . .
. L . .
. . . .
)");

inline constexpr auto RotationsJ = MakeRotationsArray(
    R"(
J . . .
J J J .
. . . .
. . . .
)",
    R"(
. J J .
. J . .
. J . .
. . . .
)",
    R"(
. . . .
J J J .
. . J .
. . . .
)",
    R"(
. J . .
. J . .
J J . .
. . . .
)");

inline constexpr auto RotationsZ = MakeRotationsArray(
    R"(
Z Z . .
. Z Z .
. . . .
. . . .
)",
    R"(
. . Z .
. Z Z .
. Z . .
. . . .
)",
    R"(
. . . .
Z Z . .
. Z Z .
. . . .
)",
    R"(
. Z . .
Z Z . .
Z . . .
. . . .
)");

inline constexpr std::array
    Prefabs{&RotationsI, &RotationsS, &RotationsO, &RotationsT, &RotationsL, &RotationsJ, &RotationsZ};

[[nodiscard]] constexpr const BlockPrefab& GetBlockPrefab(size_t block_index, size_t block_rotation)
{
    return Prefabs.at(block_index)->at(block_rotation);
}
