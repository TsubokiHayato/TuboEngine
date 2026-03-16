#include "StageManager.h"
#include "application/MapChip/MapChipField.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <format>
#include "Block/Block.h"
#include "Character/Enemy/Enemy.h"
#include "Character/Enemy/RushEnemy.h"
#include "Tile/Tile.h"
#include "Camera/FollowTopDownCamera.h"
#include "Collider/CollisionManager.h"
#include "ImGuiManager.h" // ImGui 用
#include "engine/scene/StageScene.h" // 追加: Demo判定用

void StageManager::Configure(uint32_t chunkWidth, uint32_t chunkHeight, float tileScale) {
    chunkWidth_  = chunkWidth;
    chunkHeight_ = chunkHeight;
    tileScale_   = tileScale;
}

// メタレイアウト用: intグリッドCSV読み込み (0/1/2/...)
static std::vector<std::vector<int>> LoadIntGridCsv(const std::string& path) {
    std::vector<std::vector<int>> grid;
    std::ifstream file(path);
    if (!file.is_open()) {
        return grid;
    }

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        std::stringstream ss(line);
        std::string cell;
        std::vector<int> row;
        while (std::getline(ss, cell, ',')) {
            if (cell.empty()) {
                row.push_back(0);
            } else {
                row.push_back(std::stoi(cell));
            }
        }
        grid.push_back(std::move(row));
    }
    return grid;
}

// Stage.csv から 0/1/2/... のメタレイアウトを読み込み、0 以外のセルだけチャンク生成
void StageManager::LoadMetaLayout(const std::string& metaCsvPath,
                                  Player* player,
                                  FollowTopDownCamera* followCamera) {
    // 保持プレイヤーを更新（null でなければ優先）
    if (player) {
        player_ = player;
    }
    stageInstances_.clear();

    // Demoモード時は Stage.csv を使わず、中央に 1 チャンクだけ Demo.csv を配置する
    if (StageScene::isDemoMode) {
        // 中心(0,0) に ID=1 チャンクとして Demo.csv を生成
        CreateChunkFromId(1, 0, 0, player, followCamera);
        if (player) {
            player->SetPosition(GetPlayerStartPosition());
        }
        return;
    }

    auto grid = LoadIntGridCsv(metaCsvPath);
    const int rows = static_cast<int>(grid.size());
    if (rows <= 0) {
        return;
    }

    // 各行の最大列数を取得（行ごとに列数が違っても対応）
    int maxCols = 0;
    for (const auto& row : grid) {
        maxCols = std::max<int>(maxCols, static_cast<int>(row.size()));
    }
    if (maxCols <= 0) {
        return;
    }

    // 可変サイズ Stage.csv 全体の中心セルが (0,0) 付近になるように、
    // 行・列の中心オフセットを計算する。
    // 例) 3x3 -> centerRow=1, centerCol=1 (5 が中心)
    //     3x5 -> centerRow=1, centerCol=2 (8 が中心)
    const float centerRow = (rows   - 1) * 0.5f;
    const float centerCol = (maxCols - 1) * 0.5f;

    for (int r = 0; r < rows; ++r) {
        const int cols = static_cast<int>(grid[r].size());
        for (int c = 0; c < cols; ++c) {
            int id = grid[r][c];
            // 0: 何も置かない, 1/2/...: 登録済みCSVを使ったチャンクを生成
            if (id != 0) {
                // Demoモードでない通常ステージのみここを通る

                // 元のグリッド座標(r,c)を、ステージ全体中心が (0,0) になるように平行移動
                const float relRowF = static_cast<float>(r) - centerRow;
                const float relColF = static_cast<float>(c) - centerCol;
                const int   relRow  = static_cast<int>(std::round(relRowF));
                const int   relCol  = static_cast<int>(std::round(relColF));

                CreateChunkFromId(id, relRow, relCol, player, followCamera);
            }
        }
    }

    if (player) {
        player->SetPosition(GetPlayerStartPosition());
    }
}

TuboEngine::Math::Vector3 StageManager::ComputeOriginForChunk(int row, int col) const {
    TuboEngine::Math::Vector3 origin;
    // gapScale_ でステージ同士の間隔を調整できるようにする
    origin.x = static_cast<float>(col) * static_cast<float>(chunkWidth_) * tileScale_ * gapScale_;
    origin.y = -static_cast<float>(row) * static_cast<float>(chunkHeight_) * tileScale_ * gapScale_;
    origin.z = 0.0f;
    return origin;
}

void StageManager::CreateChunkFromId(int id, int row, int col,
                                     Player* player,
                                     FollowTopDownCamera* followCamera) {
    StageInstance inst;

    // ID -> CSV パスの対応からパスを決定（見つからなければデフォルト）
    auto it = idToCsvPath_.find(id);
    if (it != idToCsvPath_.end()) {
        inst.csvPath = it->second;
    } else {
        inst.csvPath = "Resources/Stage/MapChip.csv"; // デフォルト
    }

    // Demoモード時は強制的に Demo.csv を使用
    if (StageScene::isDemoMode) {
        inst.csvPath = "Resources/Stage/Demo.csv";
    }

    inst.origin  = ComputeOriginForChunk(row, col);
    inst.visible = true;

    inst.field = std::make_unique<MapChipField>();
    inst.field->LoadMapChipCsv(inst.csvPath);
    inst.field->SetOrigin(inst.origin);

    BuildObjectsForChunk(inst, player, followCamera);

    if (inst.field) {
        inst.boundsWorld.left   = inst.origin.x;
        inst.boundsWorld.right  = inst.origin.x + MapChipField::GetBlockWidth()  * inst.field->GetNumBlockHorizontal();
        inst.boundsWorld.bottom = inst.origin.y;
        inst.boundsWorld.top    = inst.origin.y + MapChipField::GetBlockHeight() * inst.field->GetNumBlockVirtical();
    }

    stageInstances_.push_back(std::move(inst));
}

void StageManager::BuildObjectsForChunk(StageInstance& inst,
                                        Player* player,
                                        FollowTopDownCamera* followCamera) {
    if (!inst.field) return;
    MapChipField* field = inst.field.get();
    Camera* cam = followCamera ? followCamera->GetCamera() : nullptr;

    inst.playerMapX = -1;
    inst.playerMapY = -1;

    for (uint32_t y = 0; y < field->GetNumBlockVirtical(); ++y) {
        for (uint32_t x = 0; x < field->GetNumBlockHorizontal(); ++x) {
            MapChipType type = field->GetMapChipTypeByIndex(x, y);
            if (type == MapChipType::Player) {
                inst.playerMapX = static_cast<int>(x);
                inst.playerMapY = static_cast<int>(y);
            }
        }
    }

    inst.blocks.clear();
    inst.enemies.clear();
    inst.tile = std::make_unique<Tile>();

    TuboEngine::Math::Vector3 tilePos = field->GetMapChipPositionByIndex(0, 0);
    tilePos.z = -1.0f;
    inst.tile->Initialize(tilePos, {1.0f, 1.0f, 1.0f}, "tile/tile30x30.obj");
    if (cam) {
        inst.tile->SetCamera(cam);
    }
    inst.tile->Update();

    for (uint32_t y = 0; y < field->GetNumBlockVirtical(); ++y) {
        for (uint32_t x = 0; x < field->GetNumBlockHorizontal(); ++x) {
            MapChipType type = field->GetMapChipTypeByIndex(x, y);
            TuboEngine::Math::Vector3 pos = field->GetMapChipPositionByIndex(x, y);

            if (type == MapChipType::kBlock) {
                auto block = std::make_unique<Block>();
                block->Initialize(pos);
                if (cam) {
                    block->SetCamera(cam);
                }
                block->Update();
                inst.blocks.push_back(std::move(block));
            } else if (type == MapChipType::Enemy || type == MapChipType::EnemyRush) {
                auto enemy = std::make_unique<RushEnemy>();
                enemy->Initialize();
                if (cam) {
                    enemy->SetCamera(cam);
                }
                enemy->SetPlayer(player);
                enemy->SetMapChipField(field);
                enemy->SetPosition(pos);
                enemy->Update();
                inst.enemies.push_back(std::move(enemy));
            } else if (type == MapChipType::EnemyShoot) {
                auto enemy = std::make_unique<Enemy>();
                enemy->Initialize();
                if (cam) {
                    enemy->SetCamera(cam);
                }
                enemy->SetPlayer(player);
                enemy->SetMapChipField(field);
                enemy->SetPosition(pos);
                enemy->Update();
                inst.enemies.push_back(std::move(enemy));
            }
        }
    }
}

void StageManager::Update(Player* player, FollowTopDownCamera* followCamera) {
    // プレイヤーの Update はシーン/ステート側で行うため、ここでは行わない
    Camera* cam = followCamera ? followCamera->GetCamera() : nullptr;

    for (auto& inst : stageInstances_) {
        if (!inst.visible) continue;
        if (inst.tile) {
            if (cam) inst.tile->SetCamera(cam);
            inst.tile->Update();
        }
        for (auto& b : inst.blocks) {
            if (cam) b->SetCamera(cam);
            b->Update();
        }
        for (auto& e : inst.enemies) {
            if (cam) e->SetCamera(cam);
            e->SetPlayer(player);
            e->Update();
        }
    }
}

// 全チャンクの敵が全滅しているか
bool StageManager::AreAllEnemiesDefeated() const {
	bool hasAnyEnemy = false;
	for (const auto& inst : stageInstances_) {
		if (!inst.visible) continue;
		for (const auto& e : inst.enemies) {
			if (!e) continue;
			hasAnyEnemy = true;
			if (e->GetIsAlive()) {
				return false;
			}
		}
	}
	// 敵が一体もいない場合は false とする（任意仕様）
	return hasAnyEnemy;
}

// mainChunkIndex_ を一つ進める（次のチャンクが存在すれば true を返す）
bool StageManager::AdvanceToNextChunk() {
	int next = mainChunkIndex_ + 1;
	if (next < 0 || next >= static_cast<int>(stageInstances_.size())) {
		return false;
	}
	mainChunkIndex_ = next;
	return true;
}

void StageManager::Draw3D() {
    for (auto& inst : stageInstances_) {
        if (!inst.visible) continue;
        if (inst.tile) {
            inst.tile->Draw();
        }
        for (auto& b : inst.blocks) {
            b->Draw();
        }
        for (auto& e : inst.enemies) {
            e->Draw();
        }
    }
}

TuboEngine::Math::Vector3 StageManager::GetPlayerStartPosition() const {
    for (const auto& inst : stageInstances_) {
        if (!inst.field) continue;
        if (inst.playerMapX >= 0 && inst.playerMapY >= 0) {
            return inst.field->GetMapChipPositionByIndex(
                static_cast<uint32_t>(inst.playerMapX),
                static_cast<uint32_t>(inst.playerMapY));
        }
    }
    return TuboEngine::Math::Vector3{0.0f, 0.0f, 0.0f};
}

MapChipField* StageManager::GetPlayerStartField() const {
    for (const auto& inst : stageInstances_) {
        if (!inst.field) continue;
        if (inst.playerMapX >= 0 && inst.playerMapY >= 0) {
            return inst.field.get();
        }
    }
    return nullptr;
}

void StageManager::RegisterCollisions(CollisionManager* collisionManager,
                                      Player* player) {
    if (!collisionManager) return;
    if (player) {
        collisionManager->AddCollider(player);
        for (const auto& bullet : player->GetBullets()) {
            if (bullet && bullet->GetIsAlive()) {
                collisionManager->AddCollider(bullet.get());
            }
        }
    }
    for (auto& inst : stageInstances_) {
        if (!inst.visible) continue;
        for (auto& enemy : inst.enemies) {
            if (!enemy) continue;
            if (enemy->GetIsAlive() && enemy->GetHP() > 0) {
                collisionManager->AddCollider(enemy.get());
            }
        }
    }
}

void StageManager::DrawImGui() {
#ifdef USE_IMGUI
    ImGui::Begin("StageManager");

    // チャンク間隔調整スライダー
    ImGui::SliderFloat("Gap Scale", &gapScale_, 0.1f, 2.0f, "%.2f");
    ImGui::Separator();

    if (stageInstances_.empty()) {
        ImGui::Text("No stage instances.");
        ImGui::End();
        return;
    }

    ImGui::Text("Chunk Count: %d", static_cast<int>(stageInstances_.size()));
    ImGui::Separator();

    int index = 0;
    for (auto& inst : stageInstances_) {
        const float centerX = (inst.boundsWorld.left + inst.boundsWorld.right) * 0.5f;
        const float centerY = (inst.boundsWorld.bottom + inst.boundsWorld.top) * 0.5f;
        ImGui::PushID(index);
        if (ImGui::TreeNode("Chunk", "Chunk %d", index)) {
            ImGui::Text("CSV: %s", inst.csvPath.c_str());
            ImGui::Checkbox("Visible", &inst.visible);
            ImGui::Text("Origin: (%.1f, %.1f)", inst.origin.x, inst.origin.y);
            ImGui::Text("Bounds: L=%.1f R=%.1f B=%.1f T=%.1f",
                        inst.boundsWorld.left, inst.boundsWorld.right,
                        inst.boundsWorld.bottom, inst.boundsWorld.top);
            ImGui::Text("Center: (%.1f, %.1f)", centerX, centerY);
            ImGui::Text("Blocks: %d  Enemies: %d",
                        static_cast<int>(inst.blocks.size()),
                        static_cast<int>(inst.enemies.size()));
            ImGui::TreePop();
        }
        ImGui::PopID();
        ++index;
    }

    ImGui::Text("Main Chunk Index: %d", mainChunkIndex_);

    // チャンクごとの敵数/生存数を表示
    const auto enemyInfos = GetChunkEnemyInfos();
    for (size_t i = 0; i < stageInstances_.size(); ++i) {
        const auto& inst = stageInstances_[i];
        ImGui::Separator();
        ImGui::Text("Chunk %zu (id=%s) visible=%s", i, inst.csvPath.c_str(), inst.visible ? "true" : "false");
        if (i < enemyInfos.size()) {
            ImGui::Text("  Enemies: total=%d, alive=%d", enemyInfos[i].total, enemyInfos[i].alive);
        }
    }

    ImGui::End();
#endif // USE_IMGUI
}

// デバッグ用: 各チャンクの敵数・生存数を取得
std::vector<StageManager::ChunkEnemyInfo> StageManager::GetChunkEnemyInfos() const {
	std::vector<ChunkEnemyInfo> result;
	result.reserve(stageInstances_.size());
	for (const auto& inst : stageInstances_) {
		ChunkEnemyInfo info{};
		for (const auto& e : inst.enemies) {
			if (!e) continue;
			++info.total;
			if (e->GetIsAlive()) {
				++info.alive;
			}
		}
		result.push_back(info);
	}
	return result;
}
