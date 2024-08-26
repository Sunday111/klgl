#pragma once

#include "klgl/opengl/object_deleter.hpp"

namespace klgl
{

// Unique OpenGl object controlled by id
// - Constructed from object id
// - non-copiable
// - movable
// - deletes actual object on destroy
template <typename Identifier>
class GlObject
{
public:
    GlObject() noexcept = default;
    explicit GlObject(const GlObject&) = delete;
    GlObject(GlObject&& other) noexcept { MoveFrom(other); }
    GlObject& operator=(const GlObject&) = delete;
    GlObject& operator=(GlObject&& other) noexcept
    {
        MoveFrom(other);
        return *this;
    }
    ~GlObject() { ClearThis(); }

    const Identifier& GetId() const noexcept { return id_; }
    [[nodiscard]] bool IsValid() const noexcept { return id_.IsValid(); }

    [[nodiscard]] std::optional<OpenGlError> Reset() noexcept
    {
        if (id_.IsValid())
        {
            return GlObjectDeleter<Identifier>{}(std::exchange(id_, Identifier{}));
        }

        return std::nullopt;
    }

    [[nodiscard]] static GlObject CreateFrom(Identifier id)
    {
        GlObject r;
        r.id_ = id;
        return r;
    }

    operator Identifier() const  // NOLINT
    {
        return id_;
    }

protected:
    void ClearThis() noexcept
    {
        [[maybe_unused]] std::optional<OpenGlError> err = Reset();
        // Most likely you have double resource deletion you your code. This place doesn't throw an exception because
        // double delete of opengl handle does not have really bad consequences and program can continue functioning and
        // it is likely that this happens in destructor (which is the worst place for exception). But it is an assert
        // because you still have to take a look at that
        assert(!err.has_value());
    }

    void MoveFrom(GlObject& other) noexcept
    {
        ClearThis();
        id_ = other.id_;
        other.id_ = {};
    }

private:
    Identifier id_{};
};
}  // namespace klgl
