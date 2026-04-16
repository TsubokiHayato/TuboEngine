#include "StageManager.h"
#include "application/MapChip/MapChipField.h"
#include "TextureManager.h"
#include "ParticleManager.h"
#include "engine/graphic/Particle/Effects/Ring/RingEmitter.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <format>
#include "Block/Block.h"
#include "Character/Enemy/Enemy.h"
#include "Character/Enemy/RushEnemy.h"
#include "Character/Enemy/MortarEnemy.h"
#include "Character/Enemy/CircusEnemy.h"
#include "Tile/Tile.h"
#include "Camera/FollowTopDownCamera.h"
#include "Collider/CollisionManager.h"
#include "ImGuiManager.h" 
#include "engine/scene/StageScene.h" 
#include "engine/graphic/data/InstanceData.h"
#include <map>
#include <unordered_map>

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
    if (player) {
        player_ = player;
    }
    stageInstances_.clear();

    // Demoモード時は Stage.csv を使わず、中央に 1 チャンクだけ Demo.csv を配置する
    if (StageScene::isDemoMode) {
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

    int maxCols = 0;
    for (const auto& row : grid) {
        maxCols = std::max<int>(maxCols, static_cast<int>(row.size()));
    }
    if (maxCols <= 0) {
        return;
    }

    const float centerRow = (rows   - 1) * 0.5f;
    const float centerCol = (maxCols - 1) * 0.5f;

    for (int r = 0; r < rows; ++r) {
        const int cols = static_cast<int>(grid[r].size());
        for (int c = 0; c < cols; ++c) {
            int id = grid[r][c];
            if (id != 0) {
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
    origin.x = static_cast<float>(col) * static_cast<float>(chunkWidth_) * tileScale_ * gapScale_;
    origin.y = -static_cast<float>(row) * static_cast<float>(chunkHeight_) * tileScale_ * gapScale_;
    origin.z = 0.0f;
    return origin;
}

void StageManager::CreateChunkFromId(int id, int row, int col,
                                     Player* player,
                                     FollowTopDownCamera* followCamera) {
    StageInstance inst;

    auto it = idToCsvPath_.find(id);
    if (it != idToCsvPath_.end()) {
        inst.csvPath = it->second;
    } else {
        inst.csvPath = "Resources/Stage/MapChip.csv"; 
    }

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
	TuboEngine::Camera* cam = followCamera ? followCamera->GetCamera() : nullptr;

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
    inst.tiles.clear();

    for (uint32_t y = 0; y < field->GetNumBlockVirtical(); ++y) {
        for (uint32_t x = 0; x < field->GetNumBlockHorizontal(); ++x) {
            TuboEngine::Math::Vector3 tilePos = field->GetMapChipPositionByIndex(x, y);
            tilePos.z = -1.0f;
            auto tile = std::make_unique<Tile>();
            tile->Initialize(tilePos, {1.0f, 1.0f, 1.0f}, "tile/tile.obj");
            if (cam) {
                tile->SetCamera(cam);
            }
            tile->Update();
            inst.tiles.push_back(std::move(tile));

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
            } else if (type == MapChipType::EnemyMortar) {
                auto enemy = std::make_unique<MortarEnemy>();
                enemy->Initialize();
                if (cam) {
                    enemy->SetCamera(cam);
                }
                enemy->SetPlayer(player);
                enemy->SetMapChipField(field);
                enemy->SetPosition(pos);
                enemy->Update();
                inst.enemies.push_back(std::move(enemy));
            } else if (type == MapChipType::EnemyCircus) {
                auto enemy = std::make_unique<CircusEnemy>();
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
	TuboEngine::Camera* cam = followCamera ? followCamera->GetCamera() : nullptr;
    globalTimer_ += 0.016f;

    for (auto& inst : stageInstances_) {
        if (!inst.visible) continue;
        for (auto& t : inst.tiles) {
            if (cam) t->SetCamera(cam);
            t->Update();
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

        // クリア判定: 敵が全滅したらフラグを立てる
        if (!inst.isCleared) {
            bool hasEnemy = false;
            bool anyAlive = false;
            for (auto& e : inst.enemies) {
                if (e) {
                    hasEnemy = true;
                    if (e->GetIsAlive()) {
                        anyAlive = true;
                        break;
                    }
                }
            }
            if (hasEnemy && !anyAlive) {
                inst.isCleared = true;
            }
        }

        // クリア演出アニメーション更新 (1枚ずつ広がるようにするため最大値を大きめに)
        if (inst.isCleared) {
            const float kAnimSpeed = 0.02f; 
            inst.clearAnimationT += kAnimSpeed;
            if (inst.clearAnimationT > 5.0f) { // 波が端まで届くのに十分な時間
                inst.clearAnimationT = 5.0f;
            }
        }
    }

	UpdateEntranceExitEffects();
}

void StageManager::UpdateEntranceExitEffects() {
	if (stageInstances_.empty()) {
		return;
	}
	if (mainChunkIndex_ < 0 || mainChunkIndex_ >= static_cast<int>(stageInstances_.size())) {
		return;
	}

	if (effectChunkIndex_ != mainChunkIndex_) {
		effectChunkIndex_ = mainChunkIndex_;
		for (auto* e : entranceEmitters_) {
			if (e) {
				auto& p = e->GetPreset();
				p.autoEmit = false;
				e->ClearAll();
			}
		}
	}

	auto& inst = stageInstances_[mainChunkIndex_];
	auto* field = inst.field.get();
	if (!field) {
		return;
	}

    auto entrancePos = field->GetChipPositions(MapChipType::kEntrance);
    const auto exitPos = field->GetChipPositions(MapChipType::kExit);

    if (entrancePos.empty()) {
        if (inst.playerMapX >= 0 && inst.playerMapY >= 0) {
            entrancePos.push_back(field->GetMapChipPositionByIndex(
                static_cast<uint32_t>(inst.playerMapX),
                static_cast<uint32_t>(inst.playerMapY)));
        } else {
            entrancePos.push_back(inst.origin);
        }
    }

	auto EnsureEmitters = [&](std::vector<IParticleEmitter*>& list, size_t needCount, const char* baseName,
		const TuboEngine::Math::Vector4& colStart, const TuboEngine::Math::Vector4& colEnd) {
		while (list.size() < needCount) {
			TuboEngine::TextureManager::GetInstance()->LoadTexture("gradationLine.png");
			ParticlePreset p{};
			p.name = baseName;
			p.texture = "gradationLine.png";
			p.maxInstances = 64;
			p.autoEmit = true;
			p.emitRate = 1.5f;
			p.lifeMin = 0.55f;
			p.lifeMax = 0.85f;
			p.billboard = false;              
			p.simulateInWorldSpace = true;
			p.gravity = {0.0f, 0.0f, 0.0f};
			p.velMin = {0.0f, 0.0f, 0.0f};
			p.velMax = {0.0f, 0.0f, 0.0f};
			p.scaleStart = {0.35f, 0.35f, 1.0f};
			p.scaleEnd = {0.60f, 0.60f, 1.0f};
			p.colorStart = colStart;
			p.colorEnd = colEnd;
			p.center = {0.0f, 0.0f, 0.05f};
			IParticleEmitter* created = TuboEngine::ParticleManager::GetInstance()->CreateEmitter<RingEmitter>(p);
			list.push_back(created);
		}

		for (size_t i = needCount; i < list.size(); ++i) {
			if (!list[i]) continue;
			auto& preset = list[i]->GetPreset();
			preset.autoEmit = false;
			preset.center = {0.0f, 0.0f, -10000.0f};
			list[i]->ClearAll();
		}
	};

	EnsureEmitters(entranceEmitters_, entrancePos.size(), "EntranceRing", {0.4f, 0.9f, 1.0f, 0.75f}, {0.4f, 0.9f, 1.0f, 0.0f});
	EnsureEmitters(exitEmitters_, exitPos.size(), "ExitRing", {1.0f, 0.4f, 0.9f, 0.75f}, {1.0f, 0.4f, 0.9f, 0.0f});

	for (size_t i = 0; i < entrancePos.size() && i < entranceEmitters_.size(); ++i) {
		if (!entranceEmitters_[i]) continue;
		auto& preset = entranceEmitters_[i]->GetPreset();
		preset.autoEmit = true;
		preset.center = entrancePos[i] + TuboEngine::Math::Vector3{0.0f, 0.0f, 0.05f};
	}
	for (size_t i = 0; i < exitPos.size() && i < exitEmitters_.size(); ++i) {
		if (!exitEmitters_[i]) continue;
		auto& preset = exitEmitters_[i]->GetPreset();
		preset.autoEmit = true;
		preset.center = exitPos[i] + TuboEngine::Math::Vector3{0.0f, 0.0f, 0.05f};
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
	return hasAnyEnemy;
}

// mainChunkIndex_ を一つ進める
bool StageManager::AdvanceToNextChunk() {
	int next = mainChunkIndex_ + 1;
	if (next < 0 || next >= static_cast<int>(stageInstances_.size())) {
		return false;
	}
	mainChunkIndex_ = next;
	return true;
}

void StageManager::Draw3D() {
    temporaryBuffers_.clear();

    struct BatchInfo {
        std::vector<TuboEngine::InstanceData> instances;
        TuboEngine::Object3d* representative = nullptr;
    };
    std::unordered_map<TuboEngine::Model*, BatchInfo> instancedDataMap;
    
    TuboEngine::Math::Vector3 camPos = player_ ? player_->GetPosition() : TuboEngine::Math::Vector3{0, 0, 0};
    const float kCullingRadiusSq = 150.0f * 150.0f; 

    auto collectData = [&](StageInstance& inst, auto& list) {
        for (auto& item : list) {
            TuboEngine::Math::Vector3 pos = item->GetPosition();
            float distSq = (pos.x - camPos.x) * (pos.x - camPos.x) + (pos.y - camPos.y) * (pos.y - camPos.y);
            if (distSq > kCullingRadiusSq) continue;

            TuboEngine::Object3d* obj = item->GetObject3d();
            if (!obj) continue;
            TuboEngine::Model* model = obj->GetModel();
            if (!model) continue;

            TuboEngine::TransformationMatrix* matrixData = obj->GetTransformationMatrixData();
            if (!matrixData) continue;

            TuboEngine::InstanceData data;
            data.WVP = matrixData->WVP;
            data.World = matrixData->World;

            // --- Radial Synthwave Wave ---
            TuboEngine::Math::Vector3 center = {
                (inst.boundsWorld.left + inst.boundsWorld.right) * 0.5f,
                (inst.boundsWorld.bottom + inst.boundsWorld.top) * 0.5f,
                0.0f
            };
            float distance = std::sqrt((pos.x - center.x) * (pos.x - center.x) + (pos.y - center.y) * (pos.y - center.y));
            
            // 波の伝播パラメータ
            const float waveSpeed = 100.0f; // 広がる速さ
            const float fadeWidth = 20.0f;  // 色が変わるときの境界のぼかし幅
            
            // このタイル個別の進行度 (0.0 ~ 1.0)
            float localTimer = inst.clearAnimationT * waveSpeed;
            float localT = std::clamp((localTimer - distance) / fadeWidth, 0.0f, 1.0f);
            
            // 通常時: 明るめの紫
            TuboEngine::Math::Vector4 baseColor = {0.25f, 0.15f, 0.45f, 1.0f}; 
            // クリア時: マゼンタ
            TuboEngine::Math::Vector4 clearColor = {1.0f, 0.2f, 0.9f, 1.0f}; 
            
            // 基本の色補間
            data.Color.x = baseColor.x + (clearColor.x - baseColor.x) * localT;
            data.Color.y = baseColor.y + (clearColor.y - baseColor.y) * localT;
            data.Color.z = baseColor.z + (clearColor.z - baseColor.z) * localT;
            data.Color.w = 1.0f;

            // クリア後のパルス (localT が 1.0 に達したタイルのみ)
            if (localT >= 1.0f) {
                float pulse = (std::sin(globalTimer_ * 3.0f + distance * 0.1f) * 0.5f + 0.5f) * 0.5f; 
                TuboEngine::Math::Vector4 pulseColor = {0.0f, 1.0f, 1.0f, 0.0f}; // シアン
                data.Color.x += pulseColor.x * pulse;
                data.Color.y += pulseColor.y * pulse;
                data.Color.z += pulseColor.z * pulse;
                data.Color.x *= 1.3f;
                data.Color.y *= 1.3f;
                data.Color.z *= 1.3f;
            }
            
            auto& batch = instancedDataMap[model];
            batch.instances.push_back(data);
            if (!batch.representative) {
                batch.representative = obj; 
            }
        }
    };

    for (auto& inst : stageInstances_) {
        if (!inst.visible) continue;
        collectData(inst, inst.tiles);
        collectData(inst, inst.blocks);
        for (auto& e : inst.enemies) {
            if (e) e->Draw();
        }
    }

    auto commandList = TuboEngine::DirectXCommon::GetInstance()->GetCommandList();

    for (auto& [model, info] : instancedDataMap) {
        if (info.instances.empty() || !info.representative) continue;

        auto rep = info.representative;
        if (!rep->GetCubeMapFilePath().empty()) {
            commandList->SetGraphicsRootDescriptorTable(3, TuboEngine::TextureManager::GetInstance()->GetSrvHandleGPU(rep->GetCubeMapFilePath()));
        }
        commandList->SetGraphicsRootConstantBufferView(4, rep->GetDirectionalLightResource()->GetGPUVirtualAddress());
        commandList->SetGraphicsRootConstantBufferView(5, rep->GetCameraForGPUResource()->GetGPUVirtualAddress());
        commandList->SetGraphicsRootConstantBufferView(6, rep->GetLightTypeResource()->GetGPUVirtualAddress());
        commandList->SetGraphicsRootConstantBufferView(7, rep->GetPointLightResource()->GetGPUVirtualAddress());
        commandList->SetGraphicsRootConstantBufferView(8, rep->GetSpotLightResource()->GetGPUVirtualAddress());

        size_t bufferSize = sizeof(TuboEngine::InstanceData) * info.instances.size();
        auto instanceBuffer = TuboEngine::DirectXCommon::GetInstance()->CreateBufferResource(bufferSize);
        void* pData = nullptr;
        instanceBuffer->Map(0, nullptr, &pData);
        std::memcpy(pData, info.instances.data(), bufferSize);
        instanceBuffer->Unmap(0, nullptr);
        temporaryBuffers_.push_back(instanceBuffer);

        auto commonBuffer = TuboEngine::DirectXCommon::GetInstance()->CreateBufferResource(sizeof(int32_t));
        int32_t* pCommon = nullptr;
        commonBuffer->Map(0, nullptr, reinterpret_cast<void**>(&pCommon));
        *pCommon = 1;
        commonBuffer->Unmap(0, nullptr);
        temporaryBuffers_.push_back(commonBuffer);

        model->DrawInstanced(static_cast<uint32_t>(info.instances.size()), instanceBuffer->GetGPUVirtualAddress(), commonBuffer->GetGPUVirtualAddress());
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
#endif 
}

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
