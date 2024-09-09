#pragma once

#include "klgl/opengl/vertex_attribute_helper.hpp"
#include "klgl/template/class_member_traits.hpp"
#include "klgl/template/member_offset.hpp"

namespace klgl
{
template <auto MemberVariablePtr, bool normalize = false, bool to_float = true>
void RegisterAttribute(const size_t location)
{
    using MemberTraits = klgl::ClassMemberTraits<decltype(MemberVariablePtr)>;
    using AttribHelper = VertexBufferHelperStatic<typename MemberTraits::Member, normalize, to_float>;
    AttribHelper::EnableVertexAttribArray(location);
    AttribHelper::AttributePointer(
        location,
        sizeof(typename MemberTraits::Class),
        klgl::MemberOffset<MemberVariablePtr>());
}
}  // namespace klgl
