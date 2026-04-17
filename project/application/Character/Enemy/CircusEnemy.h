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
    void FireSingleMissile(const TuboEngine::Math::Vector3& launchDir, float speed);

private:
    enum class AttackType {
        Burst,      // 既存の一斉射撃
        Spiral,     // 回転しながら発射
        Cross,      // 十字方向に発射
        Targeted    // プレイヤーを狙って発射
    };

    void ExecuteAttack(AttackType type);
    void UpdateAttackCycle(float dt);

private:
    std::vector<std::unique_ptr<CircusBullet>> bullets_;
    float fireTimer_ = 0.0f;
    float fireInterval_ = 5.0f;
    int missileCount_ = 32;
    int bulletMode_ = 0; // 0: Homing, 1: Around Player
    
    AttackType currentAttack_ = AttackType::Burst;
    bool autoCycle_ = true;
    float cycleTimer_ = 0.0f;
    const float kCycleInterval = 8.0f;

    // 弾の調整パラメータ
    float bulletSpeed_ = 0.6f;
    float bulletTurnSpeed_ = 0.08f;
    float bulletChaosAmp_ = 0.4f;
    float bulletChaosFreq_ = 5.0f;

    // スパイラル攻撃用
    float spiralAngle_ = 0.0f;

    // バースト連射用メンバ
    int remainingMissiles_ = 0;
    float burstTimer_ = 0.0f;
    const float kBurstDelay = 0.03f; // 1発ごとの間隔
};
