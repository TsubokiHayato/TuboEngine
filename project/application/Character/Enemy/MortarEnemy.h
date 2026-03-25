#pragma once
#include "Enemy.h"
#include "Bullet/Enemy/EnemyMissileBullet.h"

class MortarEnemy : public Enemy {
public:
    MortarEnemy() = default;
    ~MortarEnemy() override = default;

    void Initialize() override;
    void Update() override;
    void Draw() override;

    void SetMissileInterval(float sec) { missileInterval_ = sec; }
    void SetMissileSpawnHeight(float height) { missileSpawnHeight_ = height; }
    void SetMissileImpactRadius(float radius) { missileImpactRadius_ = radius; }

private:
    void TryFireMissile(bool canSeePlayer, float dt);

private:
    std::unique_ptr<EnemyMissileBullet> missile_;
    float missileTimer_ = 0.0f;
    float missileInterval_ = 2.5f;
    float missileSpawnHeight_ = 15.0f;
    float missileImpactRadius_ = 2.0f;
};
