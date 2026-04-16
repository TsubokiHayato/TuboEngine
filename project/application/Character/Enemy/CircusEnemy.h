#pragma once
#include "Enemy.h"
#include "Bullet/Enemy/CircusBullet.h"
#include <vector>
#include <memory>

class CircusEnemy : public Enemy {
public:
    CircusEnemy() = default;
    ~CircusEnemy() override = default;

    void Initialize() override;
    void Update() override;
    void Draw() override;
    void DrawImGui();

private:
    void TryFireMissiles(bool canSeePlayer, float dt);
    void FireSingleMissile();

private:
    std::vector<std::unique_ptr<CircusBullet>> bullets_;
    float fireTimer_ = 0.0f;
    float fireInterval_ = 5.0f;
    int missileCount_ = 24;
    int bulletMode_ = 0; // 0: Homing, 1: Around Player

    // バースト連射用メンバ
    int remainingMissiles_ = 0;
    float burstTimer_ = 0.0f;
    const float kBurstDelay = 0.03f; // 1発ごとの間隔
};
