#pragma once
#include <memory>
#include <string>
#include <vector>
#include "Vector3.h"
#include "Collider/CollisionManager.h"

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

    void LoadMetaLayout(const std::string& metaCsvPath,
                        Player* player,
                        FollowTopDownCamera* followCamera);

    void Update(Player* player, FollowTopDownCamera* followCamera);

    void Draw3D();

    TuboEngine::Math::Vector3 GetPlayerStartPosition() const;

    void RegisterCollisions(CollisionManager* collisionManager,
                            Player* player);

    const std::vector<StageInstance>& GetStageInstances() const { return stageInstances_; }

private:
    void CreateChunkFromId(int id, int row, int col,
                           Player* player,
                           FollowTopDownCamera* followCamera);

    TuboEngine::Math::Vector3 ComputeOriginForChunk(int row, int col) const;

    void BuildObjectsForChunk(StageInstance& inst,
                              Player* player,
                              FollowTopDownCamera* followCamera);

private:
    std::vector<StageInstance> stageInstances_;

    uint32_t chunkWidth_  = 100;
    uint32_t chunkHeight_ = 100;
    float    tileScale_   = 30.0f;

    int mainChunkIndex_ = 0;
};
