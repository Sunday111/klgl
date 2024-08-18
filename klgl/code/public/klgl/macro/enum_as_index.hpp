#pragma once

#include "ass/enum/enum_as_index_magic_enum.hpp"  // IWYU pragma: keep (macro uses types from this header)

#if defined(KLGL_ENUM_AS_INDEX) || defined(KLGL_ENUM_AS_INDEX_ZERO_TO_KMAX)
#error "Macro redefinition or name collision"
#endif

#define KLGL_ENUM_AS_INDEX_MAGIC_ENUM(Type)                                                      \
    template <>                                                                                  \
    struct ass::EnumIndexConverter<klgl::Type> : public EnumIndexConverter_MagicEnum<klgl::Type> \
    {                                                                                            \
    }
