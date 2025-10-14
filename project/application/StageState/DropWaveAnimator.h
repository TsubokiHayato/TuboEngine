#pragma once
#include <vector>
#include "Vector3.h"
#include <memory>

class Block;
class Enemy;
class Player;

class DropWaveAnimation {
public:
    void Initialize(const std::vector<Vector3>& blockTargets, const Vector3& playerTarget, const std::vector<Vector3>& enemyTargets, int playerMapX, int playerMapY);
    void Update(float deltaTime);
    void ApplyPositions(std::vector<std::unique_ptr<Block>>& blocks, Player* player, std::vector<std::unique_ptr<Enemy>>& enemies);
    bool IsFinished() const;
private:
    // レイヤー情報、タイマー、進行状態など
};