#pragma once

#include "klgl/macro/enum_as_index.hpp"

namespace klgl
{
enum class Axis
{
    X,  // Forward
    Y,  // Right
    Z,  // Up
};

}

KLGL_ENUM_AS_INDEX_MAGIC_ENUM(Axis);
