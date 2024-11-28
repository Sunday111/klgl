#include <imgui.h>

#include <EverydayTools/Math/Math.hpp>
#include <ass/enum_map.hpp>
#include <random>

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

[[nodiscard]] constexpr const BlockPrefab& GetBlockPrefab(size_t block_index, size_t block_rotation)
{
    return Prefabs.at(block_index)->at(block_rotation);
}

using BlockId = edt::TaggedIdentifier<tags::BlockIDTag, uint32_t>;
inline constexpr BlockId kInvalidBlockId{};

class TetrisCell
{
public:
    edt::Vec4u8 color = colors::kBlack;
    BlockId block_id = kInvalidBlockId;
};

enum class KeyboardKey
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
    void Initialize() override
    {
        klgl::Application::Initialize();
        klgl::OpenGl::SetClearColor({});
        GetWindow().SetSize(500, 1000);
        GetWindow().SetTitle("Tetris Example");
        SetTargetFramerate(60);
        painter_ = std::make_unique<klgl::Painter2d>();
        klgl::ErrorHandling::Ensure(SpawnNewBlock(), "Failed to spawn the very first block.");
    }

    [[nodiscard]] bool SpawnNewBlock()
    {
        Block new_block{
            .prefab_idx = prefab_dist_(rng_),
            .rotation_idx = rotation_dist_(rng_),
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

    void Tick() override
    {
        UpdateKeyboardState();

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

        const float time = GetTimeSeconds();

        // Block movement down by gravity
        if constexpr (kGravityEnabled)
        {
            bool need_new_block = false;
            if (const float delta_time = time - last_move_down_time_; delta_time > 0.5f)
            {
                last_move_down_time_ = time;

                // Try to move the current block if it exists
                if (current_block_id_.IsValid())
                {
                    auto [new_block_id, success] = tetris_grid_.MoveBlock(current_block_id_, {0, -1});
                    current_block_id_ = new_block_id;
                    need_new_block = !success;
                }
                else
                {
                    need_new_block = true;
                }
            }

            if (need_new_block)
            {
                [[maybe_unused]] bool val = SpawnNewBlock();
            }
        }

        auto handle_move_key = [&](KeyboardKey key, edt::Vec2<int> delta)
        {
            const auto& state = keyboard_key_to_state_.Get(key);
            if (!state.is_down)
            {
                if (state.changed_state_this_frame)
                {
                    moves_on_hold_counters_.GetOrAdd(key) = 0;
                }

                return;
            }

            // Key is being held right now.
            // Determine how much time

            // On press make the first move instantly
            if (state.changed_state_this_frame)
            {
                auto [new_block_id, success] = tetris_grid_.MoveBlock(current_block_id_, delta);
                current_block_id_ = new_block_id;
                return;
            }

            // Then wait 0.5s before fast moving begins
            constexpr float delay = 0.5f;

            const float fast_move_duration = time - (state.down_since + delay);
            if (std::signbit(fast_move_duration)) return;

            constexpr float fast_move_dt = 0.25f;
            size_t& moves_counter = moves_on_hold_counters_.GetOrAdd(key);
            size_t expected_moves = static_cast<size_t>(fast_move_duration / fast_move_dt);
            assert(expected_moves >= moves_counter);

            if (expected_moves == moves_counter) return;

            moves_counter = expected_moves;
            auto [new_block_id, success] = tetris_grid_.MoveBlock(current_block_id_, delta);
            current_block_id_ = new_block_id;
        };

        handle_move_key(KeyboardKey::A, {-1, 0});
        handle_move_key(KeyboardKey::D, {1, 0});
        handle_move_key(KeyboardKey::S, {0, -1});
        if constexpr (kMoveUpAllowed) handle_move_key(KeyboardKey::W, {0, 1});

        painter_->BeginDraw();

        auto rect_size = edt::Vec2f(2.f, 2.f) / tetris_grid_.kSize.Cast<float>();
        for (auto coords : tetris_grid_.AllCoords())
        {
            painter_->DrawRect(
                {.center = {edt::Vec2f{-1, -1} + rect_size * coords.Cast<float>() + rect_size / 2},
                 .size = rect_size,
                 .color = tetris_grid_.GetCell(coords).color});
        }

        painter_->EndDraw();
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

    static constexpr bool kMoveUpAllowed = false;
    static constexpr bool kGravityEnabled = true;

    BlockId current_block_id_;
    TetrisGrid tetris_grid_;
    std::unique_ptr<klgl::Painter2d> painter_;

    size_t seed_ = 0;
    std::mt19937 rng_{seed_};
    std::uniform_int_distribution<size_t> prefab_dist_{0, Prefabs.size() - 1};
    std::uniform_int_distribution<size_t> rotation_dist_{0, 3};

    ass::EnumMap<KeyboardKey, KeyboardKeyState> keyboard_key_to_state_;
    ass::EnumMap<KeyboardKey, size_t> moves_on_hold_counters_;

    float last_move_down_time_ = 0;
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
