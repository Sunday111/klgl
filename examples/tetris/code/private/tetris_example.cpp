#include <imgui.h>

#include <EverydayTools/Math/Math.hpp>
#include <ass/enum_map.hpp>
#include <random>
#include <variant>

#include "ankerl/unordered_dense.h"
#include "ass/enum/enum_as_index_magic_enum.hpp"
#include "klgl/application.hpp"
#include "klgl/error_handling.hpp"
#include "klgl/opengl/gl_api.hpp"
#include "klgl/rendering/painter2d.hpp"
#include "klgl/template/tagged_id_hash.hpp"
#include "klgl/window.hpp"
#include "prefabs.hpp"

namespace tags
{
struct BlockIDTag;
}  // namespace tags

namespace colors
{
inline constexpr edt::Vec4u8 kBlack{0, 0, 0, 255};
inline constexpr edt::Vec4u8 kRed{255, 0, 0, 255};
inline constexpr edt::Vec4u8 kWhite{255, 255, 255, 255};
}  // namespace colors

class Block
{
public:
    size_t prefab_idx = 0;
    size_t rotation_idx = 0;
    edt::Vec2<int> pos{0, 0};
    edt::Vec4u8 color = colors::kRed;
};

using BlockId = edt::TaggedIdentifier<tags::BlockIDTag, uint32_t>;
inline constexpr BlockId kInvalidBlockId{};

class TetrisCell
{
public:
    edt::Vec4u8 color = colors::kBlack;
    BlockId block_id = kInvalidBlockId;
};

enum class KeyboardKey : uint8_t
{
    W,
    A,
    S,
    D,
    E,
    Q,
};

template <>
struct ass::EnumIndexConverter<KeyboardKey> : ass::EnumIndexConverter_MagicEnum<KeyboardKey>
{
};

class KeyboardKeyState
{
public:
    float down_since = 0.f;
    float up_since = 0.f;
    bool is_down = false;
    bool changed_state_this_frame = false;
};

inline constexpr auto kKeyToImGuiKey = []
{
    ass::EnumMap<KeyboardKey, ImGuiKey> map;

    auto add = [&map](const KeyboardKey key, const ImGuiKey value)
    {
        klgl::ErrorHandling::Ensure(!map.Contains(key), "Trying to add key to the map twice.");
        map.GetOrAdd(key) = value;
    };

    add(KeyboardKey::W, ImGuiKey_W);
    add(KeyboardKey::A, ImGuiKey_A);
    add(KeyboardKey::S, ImGuiKey_S);
    add(KeyboardKey::D, ImGuiKey_D);
    add(KeyboardKey::E, ImGuiKey_E);
    add(KeyboardKey::Q, ImGuiKey_Q);

    return map;
}();

class TetrisGrid
{
public:
    // Check if the given coordinates are inside the grid
    [[nodiscard]] constexpr bool IsInside(edt::Vec2<int> coord) const
    {
        return coord.x() >= 0 && coord.y() >= 0 && static_cast<size_t>(coord.x()) < kSize.x() &&
               static_cast<size_t>(coord.y()) < kSize.y();
    }

    // Get the coordinates of the block in the grid
    [[nodiscard]] auto BlockGridCoords(const BlockPrefab& prefab, edt::Vec2<int> block_pos) const
    {
        return prefab.AllCoords() |
               std::views::transform([block_pos](edt::Vec2<size_t> c) { return block_pos + c.Cast<int>(); });
    }

    // Check if all cells are valid and have the expected block ID
    bool AllCellsValidAndHaveExpectedBlockId(
        const BlockPrefab& prefab,
        const edt::Vec2<int>& block_pos,
        BlockId expected_block_id) const
    {
        return std::ranges::all_of(
            BlockGridCoords(prefab, block_pos),
            [&](edt::Vec2<int> c) { return IsInside(c) && GetCell(c.Cast<size_t>()).block_id == expected_block_id; });
    }

    // Add a block to the grid
    [[nodiscard]] BlockId AddBlock(const Block& block)
    {
        const BlockPrefab& prefab = GetBlockPrefab(block.prefab_idx, block.rotation_idx);

        if (!AllCellsValidAndHaveExpectedBlockId(prefab, block.pos, kInvalidBlockId))
        {
            return kInvalidBlockId;
        }

        BlockId block_id = next_block_id;
        next_block_id = BlockId::FromValue(next_block_id.GetValue() + 1);
        blocks[block_id] = block;

        // Paint the block on the grid
        for (auto c : BlockGridCoords(prefab, block.pos))
        {
            auto& cell = GetCell(c.Cast<size_t>());
            cell.block_id = block_id;
            cell.color = block.color;
        }

        return block_id;
    }

    // Get all coordinates of the grid
    [[nodiscard]] static constexpr auto AllCoords() { return Make2dCoords(kSize); }

    // Remove a block from the grid
    [[nodiscard]] std::optional<Block> RemoveBlock(BlockId block_id)
    {
        auto it = blocks.find(block_id);
        if (it == blocks.end()) return std::nullopt;

        const Block& block = it->second;
        const BlockPrefab& prefab = GetBlockPrefab(block.prefab_idx, block.rotation_idx);

        if (!AllCellsValidAndHaveExpectedBlockId(prefab, block.pos, block_id))
        {
            return std::nullopt;
        }

        for (edt::Vec2<int> c : BlockGridCoords(prefab, block.pos))
        {
            auto& cell = GetCell(c.Cast<size_t>());
            cell.block_id = kInvalidBlockId;
            cell.color = colors::kBlack;
        }

        Block removed_block_copy = block;
        blocks.erase(it);
        return removed_block_copy;
    }

    // Replace a block with a new block
    std::tuple<BlockId, bool> ReplaceBlock(BlockId block_id, const Block& new_block)
    {
        const auto opt_old_block = RemoveBlock(block_id);
        klgl::ErrorHandling::Ensure(opt_old_block.has_value(), "Failed to remove block from the grid.");
        const Block& old_block = *opt_old_block;

        if (auto new_block_id = AddBlock(new_block); new_block_id.IsValid())
        {
            return {new_block_id, true};
        }

        BlockId reverted_block_id = AddBlock(old_block);
        klgl::ErrorHandling::Ensure(reverted_block_id.IsValid(), "Failed to add old block back to the grid.");
        return {reverted_block_id, false};
    }

    // Move a block by a delta
    std::tuple<BlockId, bool> MoveBlock(BlockId block_id, edt::Vec2<int> delta)
    {
        auto block_it = blocks.find(block_id);
        klgl::ErrorHandling::Ensure(block_it != blocks.end(), "Block not found.");

        auto new_block = block_it->second;
        new_block.pos += delta;
        return ReplaceBlock(block_id, new_block);
    }

    std::optional<size_t> FindFullRow(size_t first_row_to_test) const
    {
        for (size_t y = first_row_to_test; y < kSize.y(); ++y)
        {
            const bool full_row = std::ranges::all_of(
                std::views::iota(size_t{0}, kSize.x()),
                [&](size_t x) { return GetCell({x, y}).block_id.IsValid(); });
            if (full_row)
            {
                return y;
            }
        }

        return std::nullopt;
    }

    // Rotate a block by a delta
    std::tuple<BlockId, bool> RotateBlock(BlockId block_id, int rotation_delta)
    {
        auto block_it = blocks.find(block_id);
        klgl::ErrorHandling::Ensure(block_it != blocks.end(), "Block not found.");

        size_t r = static_cast<size_t>(rotation_delta % 4 + 4);
        auto new_block = block_it->second;
        new_block.rotation_idx = (new_block.rotation_idx + r) % 4;
        return ReplaceBlock(block_id, new_block);
    }

    // Get a cell by coordinates
    [[nodiscard]] constexpr TetrisCell& GetCell(edt::Vec2<size_t> coord)
    {
        return cells[Coord2dToIndex(coord, kSize.x())];
    }
    [[nodiscard]] constexpr const TetrisCell& GetCell(edt::Vec2<size_t> coord) const
    {
        return cells[Coord2dToIndex(coord, kSize.x())];
    }

    // Grid size
    static constexpr edt::Vec2<size_t> kSize{10, 20};
    static constexpr size_t kNumCells = kSize.x() * kSize.y();
    // Cells in the grid
    std::array<TetrisCell, kNumCells> cells;
    // Map of blocks in the grid
    ankerl::unordered_dense::map<BlockId, Block, klgl::TaggedIdentifierHash<BlockId>> blocks;
    // Next block ID to be assigned
    BlockId next_block_id = BlockId::FromValue(0);
};

class Painter2dApp : public klgl::Application
{
    struct AnimatedRect
    {
        klgl::Painter2d::Rect2d start_state{};
        klgl::Painter2d::Rect2d finish_state{};
        float start_time = 0.f;
        float duration = 0.f;
    };

    void Initialize() override
    {
        klgl::Application::Initialize();
        klgl::OpenGl::SetClearColor(0.2f, 0.2f, 0.2f, 1.f);
        GetWindow().SetSize(500, 1000);
        GetWindow().SetTitle("Tetris Example");
        SetTargetFramerate(60.f);
        painter_ = std::make_unique<klgl::Painter2d>();
        state_ = SpawnBlockState{};
    }

    [[nodiscard]] bool SpawnNewBlock()
    {
        Block new_block{
            .prefab_idx = prefab_dist_(rnd_),
            .rotation_idx = rotation_dist_(rnd_),
            .pos = {3, 16},
            .color = edt::Vec4u8(edt::Math::GetRainbowColors(GetTimeSeconds()), 255),
        };

        auto new_block_id = tetris_grid_.AddBlock(new_block);
        if (new_block_id.IsValid())
        {
            current_block_id_ = new_block_id;
            return true;
        }
        return false;
    }

    ////////////////////////////////////////////// Block fall state //////////////////////////////////////////////
    struct BlockFallState
    {
    };

    void Tick(BlockFallState&)
    {
        // Block rotations
        {
            int delta_rotation = 0;
            if (KeyReleasedThisFrame(KeyboardKey::E)) delta_rotation += 1;
            if (KeyReleasedThisFrame(KeyboardKey::Q)) delta_rotation -= 1;

            if (delta_rotation != 0 && current_block_id_.IsValid())
            {
                auto [new_block_id, success] = tetris_grid_.RotateBlock(current_block_id_, delta_rotation);
                current_block_id_ = new_block_id;
            }
        }

        // Instant block movement
        {
            auto handle_move_key = [&](KeyboardKey key, edt::Vec2<int> delta)
            {
                const auto& state = keyboard_key_to_state_.Get(key);
                if (state.is_down)
                {
                    // On press make the first move instantly
                    if (state.changed_state_this_frame)
                    {
                        auto [new_block_id, success] = tetris_grid_.MoveBlock(current_block_id_, delta);
                        current_block_id_ = new_block_id;
                    }
                }
            };

            handle_move_key(KeyboardKey::A, {-1, 0});
            handle_move_key(KeyboardKey::D, {1, 0});
            handle_move_key(KeyboardKey::S, {0, -1});
            if constexpr (kMoveUpAllowed) handle_move_key(KeyboardKey::W, {0, 1});
        }
    }

    void TimeStep(BlockFallState&, size_t time_step)
    {
        constexpr size_t kTimeStepsBetweenGravityMoves = 15;
        const bool gravity_time_step = time_step % kTimeStepsBetweenGravityMoves == 0;

        if (gravity_time_step)
        {
            // Block movement down by gravity
            if constexpr (kGravityEnabled)
            {
                // Try to move the current block if it exists
                if (current_block_id_.IsValid())
                {
                    auto [new_block_id, success] = tetris_grid_.MoveBlock(current_block_id_, {0, -1});
                    current_block_id_ = new_block_id;
                    if (!success)
                    {
                        if (auto opt_row_idx = tetris_grid_.FindFullRow(0))
                        {
                            state_ = DeleteRowsState{.row_index = *opt_row_idx};
                        }
                        else
                        {
                            state_ = SpawnBlockState{};
                        }
                    }
                }
            }
        }

        const float time = GetTimeSeconds();
        auto handle_held_key = [&](KeyboardKey key, edt::Vec2<int> delta)
        {
            const auto& state = keyboard_key_to_state_.Get(key);
            if (!state.is_down || state.changed_state_this_frame)
            {
                return;
            }

            // Then wait 0.5s before fast moving begins
            constexpr float delay = 0.5f;

            const float fast_move_duration = time - (state.down_since + delay);
            if (std::signbit(fast_move_duration)) return;

            assert(expected_moves >= moves_counter);

            auto [new_block_id, success] = tetris_grid_.MoveBlock(current_block_id_, delta);
            current_block_id_ = new_block_id;
        };

        handle_held_key(KeyboardKey::A, {-1, 0});
        handle_held_key(KeyboardKey::D, {1, 0});
        if (!gravity_time_step)
        {
            handle_held_key(KeyboardKey::S, {0, -1});
        }
    }

    ////////////////////////////////////////////// Row delete state //////////////////////////////////////////////

    struct DeleteRowsState
    {
        size_t row_index = 0;
        size_t next_x = 0;
    };

    void TimeStep(DeleteRowsState& state, [[maybe_unused]] size_t time_step)
    {
        if (state.next_x < tetris_grid_.kSize.x())
        {
            const edt::Vec2<size_t> coords{state.next_x, state.row_index};
            auto& cell = tetris_grid_.GetCell(coords);
            auto cell_color = cell.color;
            cell.block_id = kInvalidBlockId;
            cell.color = colors::kBlack;

            const float time = GetTimeSeconds();
            auto cell_rect = MakeCellRect(coords);
            constexpr size_t num_subdivisions = 4;
            cell_rect.center += cell_rect.size / static_cast<float>(num_subdivisions * 2) - cell_rect.size / 2;
            cell_rect.size /= static_cast<float>(num_subdivisions);

            std::uniform_real_distribution<float> dist_distr(0.f, cell_rect.size.x() * 5.f);

            for (const size_t kx : std::views::iota(size_t{0}, size_t{4}))
            {
                for (const size_t ky : std::views::iota(size_t{0}, size_t{4}))
                {
                    auto& anim = animations_.emplace_back();
                    anim.start_time = time;
                    anim.duration = 2.f;
                    anim.start_state = cell_rect;
                    anim.start_state.center += edt::Vec2<size_t>{kx, ky}.Cast<float>() * cell_rect.size;
                    anim.start_state.color = cell_color;

                    anim.finish_state = anim.start_state;
                    anim.finish_state.color.w() = 0;
                    anim.finish_state.center.x() += dist_distr(rnd_);
                    anim.finish_state.center.y() += dist_distr(rnd_);
                }
            }

            state.next_x++;
        }
        else
        {
            size_t row = state.row_index;
            state_ = MoveDeletedRowUp{
                .deleted_row = row,
                .current_row = row,
            };
        }
    }

    void Tick(DeleteRowsState&) {}

    ////////////////////////////////////////////// Spawn block state //////////////////////////////////////////////

    struct SpawnBlockState
    {
    };

    void TimeStep([[maybe_unused]] SpawnBlockState& state, [[maybe_unused]] size_t time_step)
    {
        if (SpawnNewBlock())
        {
            state_ = BlockFallState{};
        }
    }

    void Tick(SpawnBlockState&) {}

    ///////////////////////////////////////////// Move deleted row up /////////////////////////////////////////////

    struct MoveDeletedRowUp
    {
        size_t deleted_row = 0;
        size_t current_row = 0;
    };

    void TimeStep(MoveDeletedRowUp& state, [[maybe_unused]] size_t time_step)
    {
        size_t y = state.current_row;
        size_t next_y = y + 1;
        if (next_y < tetris_grid_.kSize.y())
        {
            for (size_t x = 0; x != tetris_grid_.kSize.x(); ++x)
            {
                std::swap(tetris_grid_.GetCell({x, y}), tetris_grid_.GetCell({x, next_y}));
            }

            state.current_row++;
        }
        else
        {
            auto opt_row = tetris_grid_.FindFullRow(state.deleted_row);
            if (opt_row.has_value())
            {
                state_ = DeleteRowsState{.row_index = *opt_row};
            }
            else
            {
                state_ = SpawnBlockState{};
            }
        }
    }

    void Tick(MoveDeletedRowUp&) {}

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void Tick() override
    {
        UpdateKeyboardState();

        const float time = GetTimeSeconds();
        constexpr float kTimeStepSeconds = 0.06f;
        const size_t current_time_step = static_cast<size_t>(time / kTimeStepSeconds);

        std::visit([&](auto& state) { Tick(state); }, state_);

        if (current_time_step != last_handled_time_step_)
        {
            std::visit([&](auto& state) { TimeStep(state, current_time_step); }, state_);
            last_handled_time_step_ = current_time_step;
        }

        painter_->BeginDraw();

        for (auto coords : tetris_grid_.AllCoords())
        {
            auto cell_rect = MakeCellRect(coords);
            if (!tetris_grid_.GetCell(coords).block_id.IsValid())
            {
                cell_rect.size *= 0.97f;
            }
            painter_->DrawRect(cell_rect);
        }

        DrawAnimatedRects();

        painter_->EndDraw();
    }

    klgl::Painter2d::Rect2d MakeCellRect(const edt::Vec2<size_t> coords) const
    {
        auto rect_size = edt::Vec2f(2.f, 2.f) / tetris_grid_.kSize.Cast<float>();
        return {
            .center = {edt::Vec2f{-1, -1} + rect_size * coords.Cast<float>() + rect_size / 2},
            .size = rect_size,
            .color = tetris_grid_.GetCell(coords).color};
    }

    void DrawAnimatedRects()
    {
        const float time = GetTimeSeconds();
        for (auto& animation : animations_)
        {
            const float k = std::clamp((time - animation.start_time) / animation.duration, 0.f, 1.f);
            const auto& a = animation.start_state;
            const auto& b = animation.finish_state;

            klgl::Painter2d::Rect2d rect{
                .center = edt::Math::Lerp(a.center, b.center, k),
                .size = edt::Math::Lerp(a.size, b.size, k),
                .color = edt::Math::Lerp(a.color.Cast<float>(), b.color.Cast<float>(), k).Cast<uint8_t>(),
                .rotation_degrees = std::lerp(a.rotation_degrees, b.rotation_degrees, k),
            };

            painter_->DrawRect(rect);
        }

        if (!animations_.empty()) fmt::println("");

        const size_t to_remove = std::size(std::ranges::partition(
            animations_,
            [&](const AnimatedRect& rect) { return (rect.start_time + rect.duration) > time; }));
        animations_.resize(animations_.size() - to_remove);
    }

    bool KeyReleasedThisFrame(KeyboardKey key) const
    {
        const auto& state = keyboard_key_to_state_.Get(key);
        return state.changed_state_this_frame && !state.is_down;
    }

    bool KeyPressedThisFrame(KeyboardKey key) const
    {
        const auto& state = keyboard_key_to_state_.Get(key);
        return state.changed_state_this_frame && state.is_down;
    }

    void UpdateKeyboardState()
    {
        // Maybe it makes sense to release all pressed keys at this point
        if (ImGui::GetIO().WantCaptureKeyboard) return;

        const float t = GetTimeSeconds();
        for (KeyboardKey key : ass::EnumSet<KeyboardKey>::Full())
        {
            auto& state = keyboard_key_to_state_.GetOrAdd(key);
            const bool is_down = ImGui::IsKeyDown(kKeyToImGuiKey.Get(key));
            state.changed_state_this_frame = is_down != state.is_down;

            if (state.changed_state_this_frame)
            {
                state.is_down = is_down;
                (is_down ? state.down_since : state.up_since) = t;
            }
        }
    }

    static constexpr bool kMoveUpAllowed = true;
    static constexpr bool kGravityEnabled = true;

    BlockId current_block_id_;
    TetrisGrid tetris_grid_;
    std::unique_ptr<klgl::Painter2d> painter_;

    unsigned seed_ = 0;
    std::mt19937 rnd_{seed_};
    std::uniform_int_distribution<size_t> prefab_dist_{0, Prefabs.size() - 1};
    std::uniform_int_distribution<size_t> rotation_dist_{0, 3};

    std::variant<SpawnBlockState, BlockFallState, DeleteRowsState, MoveDeletedRowUp> state_;

    ass::EnumMap<KeyboardKey, KeyboardKeyState> keyboard_key_to_state_;

    size_t last_handled_time_step_ = 0;

    std::vector<AnimatedRect> animations_;
};

void Main()
{
    Painter2dApp app;
    app.Run();
}

int main()
{
    klgl::ErrorHandling::InvokeAndCatchAll(Main);
    return 0;
}
