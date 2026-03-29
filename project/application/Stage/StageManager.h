#pragma once
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include "Vector3.h"
#include "Collider/CollisionManager.h"

class IParticleEmitter;

class Player;
class Enemy;
class Block;
class Tile;
class MapChipField;
class FollowTopDownCamera;

// StageScene の StageBounds 相当をここでも定義
struct StageBounds {
    float left{};
    float right{};
    float bottom{};
    float top{};
};

class StageManager {
public:
    struct StageInstance {
        std::string csvPath;
        TuboEngine::Math::Vector3 origin{0.0f, 0.0f, 0.0f};
        bool visible = true;

        std::unique_ptr<MapChipField> field;
        std::vector<std::unique_ptr<Block>> blocks;
        std::vector<std::unique_ptr<Enemy>> enemies;
        std::unique_ptr<Tile> tile;

        int playerMapX = -1;
        int playerMapY = -1;

        StageBounds boundsWorld{};
    };

public:
    StageManager() = default;

    void Configure(uint32_t chunkWidth, uint32_t chunkHeight, float tileScale);

    // プレイヤーポインタを借りる（所有はしない）
    void SetPlayer(Player* player) { player_ = player; }
    Player* GetPlayer() const { return player_; }

    // チャンクIDと使用するCSVパスを登録
    void RegisterChunkCsv(int id, const std::string& path) { idToCsvPath_[id] = path; }

    void LoadMetaLayout(const std::string& metaCsvPath,
                        Player* player,
                        FollowTopDownCamera* followCamera);

    void Update(Player* player, FollowTopDownCamera* followCamera);

    // 現在メインとなっているチャンクのインデックスを取得
    int GetMainChunkIndex() const { return mainChunkIndex_; }
    // 次のチャンクに進める（単純に mainChunkIndex_ を +1 する）
    bool AdvanceToNextChunk();

    // 全チャンクの敵が全滅しているかを判定
    bool AreAllEnemiesDefeated() const;

    void Draw3D();

    TuboEngine::Math::Vector3 GetPlayerStartPosition() const;
    MapChipField* GetPlayerStartField() const; // プレイヤー開始マスの属する Field

    void RegisterCollisions(CollisionManager* collisionManager,
                            Player* player);

    const std::vector<StageInstance>& GetStageInstances() const { return stageInstances_; }

    // ImGui で各チャンクの情報を表示
    void DrawImGui();

    // デバッグ用: 各チャンク内の敵数・生存数を取得
    struct ChunkEnemyInfo {
        int total = 0;
        int alive = 0;
    };
    std::vector<ChunkEnemyInfo> GetChunkEnemyInfos() const;

private:
    void CreateChunkFromId(int id, int row, int col,
                           Player* player,
                           FollowTopDownCamera* followCamera);

    TuboEngine::Math::Vector3 ComputeOriginForChunk(int row, int col) const;

    void BuildObjectsForChunk(StageInstance& inst,
                              Player* player,
                              FollowTopDownCamera* followCamera);

	// Entrance/Exit 表示用のパーティクルを更新
	void UpdateEntranceExitEffects();

private:
    std::vector<StageInstance> stageInstances_;

    uint32_t chunkWidth_  = 100;
    uint32_t chunkHeight_ = 100;
    float    tileScale_   = 30.0f;

    // チャンク間隔調整係数（1.0 でぴったり、0.5 で半分など）
    float gapScale_ = 0.5f;

    int mainChunkIndex_ = 0;

    // ID -> CSV パスの対応表
    std::unordered_map<int, std::string> idToCsvPath_;

    // StageScene から借りるだけのプレイヤー参照（所有しない）
    Player* player_ = nullptr;

	// Entrance / Exit 演出
	std::vector<IParticleEmitter*> entranceEmitters_;
	std::vector<IParticleEmitter*> exitEmitters_;
	int effectChunkIndex_ = -1;
};
