#pragma once

#include "ass/enum/enum_as_index.hpp"  // IWYU pragma: keep (macro use types from this header)

#if defined(KLGL_ENUM_AS_INDEX) || defined(KLGL_ENUM_AS_INDEX_ZERO_TO_KMAX)
#error "Macro redefinition or name collision"
#endif

#define KLGL_ENUM_AS_INDEX(Type, Begin, End)                                                                \
    template <>                                                                                             \
    struct ass::EnumIndexConverter<klgl::Type> : ass::EnumIndexConverter_Continuous<klgl::Type, Begin, End> \
    {                                                                                                       \
    };

#define KLGL_ENUM_AS_INDEX_ZERO_TO_KMAX(Type) \
    KLGL_ENUM_AS_INDEX(Type, static_cast<klgl::Type>(std::underlying_type_t<klgl::Type>{0}), klgl::Type::kMax)
