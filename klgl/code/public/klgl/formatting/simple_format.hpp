#pragma once

#include <fmt/core.h>
#include <fmt/format.h>

#include <type_traits>

namespace klgl
{
template <typename T>
struct IsFormattable : std::false_type
{
};

// This structure used as base class for other formatters so that they don't have
// to write parse method. This method parses format modifiers from format string
// which we usually don't need.
struct FormatterWithEmptyParse
{
    constexpr auto parse(fmt::format_parse_context& ctx)
    {
        auto i = ctx.begin(), end = ctx.end();
        if (i != end && *i != '}') throw fmt::format_error("invalid format");
        return i;
    }
};

template <typename T>
concept Formattable = IsFormattable<T>::value;

}  // namespace klgl

#ifdef KLGL_MAKE_STRUCT_FORMATTER
#error "Macrodefinition name collision"
#endif

// This makes a specialization for fmt::formatter which calls TypeName::Method to write type to text stream
// Must be used in the global namespace and first argument must contain type namespace (if any)
#define KLGL_MAKE_STRUCT_FORMATTER(TypeName, Method)                                               \
    template <>                                                                                    \
    struct klgl::IsFormattable<TypeName>                                                           \
    {                                                                                              \
        static constexpr bool kValue = true;                                                       \
    };                                                                                             \
                                                                                                   \
    template <>                                                                                    \
    struct fmt::formatter<TypeName> : klgl::FormatterWithEmptyParse                                \
    {                                                                                              \
        template <typename FmtContext>                                                             \
        constexpr auto format(const TypeName& value, FmtContext& ctx) const -> decltype(ctx.out()) \
        {                                                                                          \
            value.Method(ctx);                                                                     \
            return ctx.out();                                                                      \
        }                                                                                          \
    }
