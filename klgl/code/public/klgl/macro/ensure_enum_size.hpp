#pragma once

#include <type_traits>  // IWYU pragma: keep

#ifndef KLGL_ENSURE_ENUM_SIZE
#define KLGL_ENSURE_ENUM_SIZE(Type, ExpectedValueOfKMax) \
    static_assert(static_cast<std::underlying_type_t<Type>>(Type::kMax) == ExpectedValueOfKMax)
#else
#error
#endif
