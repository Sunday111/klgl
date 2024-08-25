#pragma once

#include <glad/glad.h>

#include <EverydayTools/Template/TaggedIdentifier.hpp>

namespace klgl::tags
{
struct GlShaderIdTag;
struct GlProgramIdTag;
struct GlBufferIdTag;
struct GlVertexArrayIdTag;
struct GlTextureIdTag;
}  // namespace klgl::tags

namespace klgl
{
using GlShaderId = edt::TaggedIdentifier<tags::GlShaderIdTag, GLuint, 0>;
using GlProgramId = edt::TaggedIdentifier<tags::GlProgramIdTag, GLuint, 0>;
using GlBufferId = edt::TaggedIdentifier<tags::GlBufferIdTag, GLuint, 0>;
using GlVertexArrayId = edt::TaggedIdentifier<tags::GlVertexArrayIdTag, GLuint, 0>;
using GlTextureId = edt::TaggedIdentifier<tags::GlTextureIdTag, GLuint, 0>;
}  // namespace klgl
