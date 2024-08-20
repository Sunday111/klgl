#pragma once

#include <bit>

#include "ass/enum_map.hpp"
#include "ass/fixed_unordered_map.hpp"
#include "magic_enum.hpp"

namespace klgl::detail
{

template <typename T, typename Enable = void>
struct ConstexprHasher
{
};

template <std::integral T>
struct ConstexprHasher<T>
{
    [[nodiscard]] constexpr size_t operator()(const T v) const
    {
        return std::bit_cast<size_t>(static_cast<int64_t>(v));
    }
};

template <std::unsigned_integral T>
struct ConstexprHasher<T>
{
    [[nodiscard]] constexpr size_t operator()(const T v) const { return v; }
};

template <typename KlglValue, typename OpenGlValue>
struct OpenGlValueConverter
{
    ass::EnumMap<KlglValue, OpenGlValue> to_gl_value;
    ass::FixedUnorderedMap<magic_enum::enum_count<KlglValue>(), OpenGlValue, KlglValue, ConstexprHasher<OpenGlValue>>
        from_gl_enum;

    constexpr void Add(KlglValue key, OpenGlValue value)
    {
        assert(!to_gl_value.Contains(key));
        to_gl_value.GetOrAdd(key) = value;

        assert(!from_gl_enum.Contains(value));
        from_gl_enum.Add(value, key);
    }
};
}  // namespace klgl::detail
