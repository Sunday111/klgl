#include <imgui.h>

#include <EverydayTools/Math/Math.hpp>
#include <random>

#include "ankerl/unordered_dense.h"
#include "klgl/application.hpp"
#include "klgl/error_handling.hpp"
#include "klgl/opengl/gl_api.hpp"
#include "klgl/rendering/painter2d.hpp"
#include "klgl/template/tagged_id_hash.hpp"
#include "klgl/window.hpp"
#include "prefabs.hpp"

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

struct BlockIDTag
{
};
using BlockId = edt::TaggedIdentifier<BlockIDTag, uint32_t>;
inline constexpr BlockId kInvalidBlockId{};

class TetrisCell
{
public:
    edt::Vec4u8 color = colors::kBlack;
    BlockId block_id = kInvalidBlockId;
};

class TetrisGrid
{
public:
    TetrisGrid() { cells.resize(size.x() * size.y()); }

    [[nodiscard]] constexpr bool IsInside(edt::Vec2<int> coord) const
    {
        return coord.x() >= 0 && coord.y() >= 0 && static_cast<size_t>(coord.x()) < size.x() &&
               static_cast<size_t>(coord.y()) < size.y();
    }

    [[nodiscard]] auto BlockGridCoords(const BlockPrefab& prefab, edt::Vec2<int> block_pos) const
    {
        return prefab.AllCoords() |
               std::views::transform([block_pos](edt::Vec2<size_t> c) { return block_pos + c.Cast<int>(); });
    }

    bool AllCellsValidAndHaveExpectedBlockId(
        const BlockPrefab& prefab,
        const edt::Vec2<int>& block_pos,
        BlockId expected_block_id) const
    {
        return std::ranges::all_of(
            BlockGridCoords(prefab, block_pos),
            [&](edt::Vec2<int> c) { return IsInside(c) && GetCell(c.Cast<size_t>()).block_id == expected_block_id; });
    }

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

    [[nodiscard]] constexpr auto AllCoords() const { return Make2dCoords(size); }

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

    std::tuple<BlockId, bool> MoveBlock(BlockId block_id, edt::Vec2<int> delta)
    {
        auto block_it = blocks.find(block_id);
        klgl::ErrorHandling::Ensure(block_it != blocks.end(), "Block not found.");

        auto new_block = block_it->second;
        new_block.pos += delta;
        return ReplaceBlock(block_id, new_block);
    }

    std::tuple<BlockId, bool> RotateBlock(BlockId block_id, int rotation_delta)
    {
        auto block_it = blocks.find(block_id);
        klgl::ErrorHandling::Ensure(block_it != blocks.end(), "Block not found.");

        size_t r = static_cast<size_t>(rotation_delta % 4 + 4);
        auto new_block = block_it->second;
        new_block.rotation_idx = (new_block.rotation_idx + r) % 4;
        return ReplaceBlock(block_id, new_block);
    }

    [[nodiscard]] constexpr TetrisCell& operator[](edt::Vec2<size_t> coord) { return GetCell(coord); }
    [[nodiscard]] constexpr const TetrisCell& operator[](edt::Vec2<size_t> coord) const { return GetCell(coord); }

    [[nodiscard]] constexpr TetrisCell& GetCell(edt::Vec2<size_t> coord) { return cells[CoordToIndex(coord)]; }
    [[nodiscard]] constexpr const TetrisCell& GetCell(edt::Vec2<size_t> coord) const
    {
        return cells[CoordToIndex(coord)];
    }

    [[nodiscard]] constexpr size_t CoordToIndex(edt::Vec2<size_t> coord) const
    {
        return CoordToIdx(coord.x(), coord.y());
    }
    [[nodiscard]] constexpr size_t CoordToIdx(size_t x, size_t y) const { return size.x() * y + x; }

    edt::Vec2<size_t> size{10, 20};
    std::vector<TetrisCell> cells;
    ankerl::unordered_dense::map<BlockId, Block, klgl::TaggedIdentifierHash<BlockId>> blocks;
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
        const float time = GetTimeSeconds();
        if (const float delta_time = time - last_action_time_; delta_time > 0.5f)
        {
            last_action_time_ = time;
            if (!ImGui::GetIO().WantCaptureKeyboard)
            {
                if (ImGui::IsKeyDown(ImGuiKey_1))
                {
                    [[maybe_unused]] bool success = SpawnNewBlock();
                }

                int delta_rotation = 0;
                if (ImGui::IsKeyDown(ImGuiKey_E))
                {
                    delta_rotation += 1;
                }
                if (ImGui::IsKeyDown(ImGuiKey_Q))
                {
                    delta_rotation -= 1;
                }

                if (delta_rotation != 0 && current_block_id_.IsValid())
                {
                    auto [new_block_id, success] = tetris_grid_.RotateBlock(current_block_id_, delta_rotation);
                    current_block_id_ = new_block_id;
                }

                edt::Vec2<int> delta_move{0, 0};

                if (ImGui::IsKeyDown(ImGuiKey_S)) delta_move.y() -= 1;
                if (ImGui::IsKeyDown(ImGuiKey_W)) delta_move.y() += 1;
                if (ImGui::IsKeyDown(ImGuiKey_A)) delta_move.x() -= 1;
                if (ImGui::IsKeyDown(ImGuiKey_D)) delta_move.x() += 1;

                // Try to move the current block if it exists
                if (delta_move != edt::Vec2<int>{0, 0} && current_block_id_.IsValid())
                {
                    auto [new_block_id, success] = tetris_grid_.MoveBlock(current_block_id_, delta_move);
                    current_block_id_ = new_block_id;
                }
            }
        }

        painter_->BeginDraw();

        auto rect_size = edt::Vec2f(2.f, 2.f) / tetris_grid_.size.Cast<float>();
        for (auto coords : tetris_grid_.AllCoords())
        {
            painter_->DrawRect(
                {.center = {edt::Vec2f{-1, -1} + rect_size * coords.Cast<float>() + rect_size / 2},
                 .size = rect_size,
                 .color = tetris_grid_.GetCell(coords).color});
        }

        painter_->EndDraw();
    }

    BlockId current_block_id_;
    TetrisGrid tetris_grid_;
    std::unique_ptr<klgl::Painter2d> painter_;

    size_t seed_ = 0;
    std::mt19937 rng_{seed_};
    std::uniform_int_distribution<size_t> prefab_dist_{0, Prefabs.size() - 1};
    std::uniform_int_distribution<size_t> rotation_dist_{0, 3};

    float last_action_time_ = 0;
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
