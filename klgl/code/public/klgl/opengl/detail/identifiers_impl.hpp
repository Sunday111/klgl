#include <string_view>

#include "klgl/opengl/identifiers.hpp"

namespace klgl::detail
{
template <typename Identifier>
struct IdTraits;

template <>
struct IdTraits<GlTextureId>
{
    static constexpr auto generator = &glGenTextures;
    static constexpr std::string_view generator_name = "glGenTextures";
};

template <>
struct IdTraits<GlBufferId>
{
    static constexpr auto generator = &glGenBuffers;
    static constexpr std::string_view generator_name = "glGenBuffers";
};

template <>
struct IdTraits<GlVertexArrayId>
{
    static constexpr auto generator = &glGenVertexArrays;
    static constexpr std::string_view generator_name = "glGenVertexArrays";
};

}  // namespace klgl::detail
