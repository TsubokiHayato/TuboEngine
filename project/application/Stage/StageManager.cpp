#include "StageManager.h"
#include "application/MapChip/MapChipField.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <format>
#include "MapChip/MapChipField.h"
#include "Block/Block.h"
#include "Character/Enemy/Enemy.h"
#include "Character/Enemy/RushEnemy.h"
#include "Tile/Tile.h"
#include "Camera/FollowTopDownCamera.h"
#include "Collider/CollisionManager.h"

void StageManager::Configure(uint32_t chunkWidth, uint32_t chunkHeight, float tileScale) {
    chunkWidth_  = chunkWidth;
    chunkHeight_ = chunkHeight;
    tileScale_   = tileScale;
}

// メタレイアウト用: intグリッドCSV読み込み (0/1 など)
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

// Stage.csv から 0/1 のメタレイアウトを読み込み、1 のセルだけ現在のステージ構成として生成
void StageManager::LoadMetaLayout(const std::string& metaCsvPath,
                                  Player* player,
                                  FollowTopDownCamera* followCamera) {
    stageInstances_.clear();

    auto grid = LoadIntGridCsv(metaCsvPath);
    const int rows = static_cast<int>(grid.size());
    for (int r = 0; r < rows; ++r) {
        const int cols = static_cast<int>(grid[r].size());
        for (int c = 0; c < cols; ++c) {
            int id = grid[r][c];
            // 0: 何も置かない, 1: MapChip.csv を使ったチャンクを生成
            if (id == 1) {
                CreateChunkFromId(1, r, c, player, followCamera);
            }
        }
    }

    if (player) {
        player->SetPosition(GetPlayerStartPosition());
    }
}

TuboEngine::Math::Vector3 StageManager::ComputeOriginForChunk(int row, int col) const {
    TuboEngine::Math::Vector3 origin;
    origin.x = static_cast<float>(col) * static_cast<float>(chunkWidth_) * tileScale_;
    origin.y = -static_cast<float>(row) * static_cast<float>(chunkHeight_) * tileScale_;
    origin.z = 0.0f;
    return origin;
}

void StageManager::CreateChunkFromId(int /*id*/, int row, int col,
                                     Player* player,
                                     FollowTopDownCamera* followCamera) {
    StageInstance inst;

    // 現在のステージ構成: すべて MapChip.csv を使用
    inst.csvPath = "Resources/Stage/MapChip.csv";

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
    inst.tile->SetCamera(cam);
    inst.tile->Update();

    for (uint32_t y = 0; y < field->GetNumBlockVirtical(); ++y) {
        for (uint32_t x = 0; x < field->GetNumBlockHorizontal(); ++x) {
            MapChipType type = field->GetMapChipTypeByIndex(x, y);
            TuboEngine::Math::Vector3 pos = field->GetMapChipPositionByIndex(x, y);

            if (type == MapChipType::kBlock) {
                auto block = std::make_unique<Block>();
                block->Initialize(pos);
                block->SetCamera(cam);
                block->Update();
                inst.blocks.push_back(std::move(block));
            } else if (type == MapChipType::Enemy || type == MapChipType::EnemyRush) {
                auto enemy = std::make_unique<RushEnemy>();
                enemy->Initialize();
                enemy->SetCamera(cam);
                enemy->SetPlayer(player);
                enemy->SetMapChipField(field);
                enemy->SetPosition(pos);
                enemy->Update();
                inst.enemies.push_back(std::move(enemy));
            } else if (type == MapChipType::EnemyShoot) {
                auto enemy = std::make_unique<Enemy>();
                enemy->Initialize();
                enemy->SetCamera(cam);
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
    Camera* cam = followCamera ? followCamera->GetCamera() : nullptr;

    for (auto& inst : stageInstances_) {
        if (!inst.visible) continue;
        if (inst.tile) {
            inst.tile->SetCamera(cam);
            inst.tile->Update();
        }
        for (auto& b : inst.blocks) {
            b->SetCamera(cam);
            b->Update();
        }
        for (auto& e : inst.enemies) {
            e->SetCamera(cam);
            e->SetPlayer(player);
            e->Update();
        }
    }
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
