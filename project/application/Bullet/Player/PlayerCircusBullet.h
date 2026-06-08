#pragma once
#include "Bullet/Player/PlayerBullet.h"
#include <vector>
#include <random>

class Enemy;

// ジャスト回避時のカウンター用弾。敵のサーカス弾と同等のホーミングと演出を持つ。
class PlayerCircusBullet : public PlayerBullet {
public:
    void Initialize(const TuboEngine::Math::Vector3& startPos) override;
    void InitializeCircus(const TuboEngine::Math::Vector3& startPos, TuboEngine::Camera* camera, class CircusEnemy* boss = nullptr);
    void Update() override;
    void Draw() override;
    void OnCollision(Collider* other) override;

    // カウンター特有の設定
    void SetTargetEnemyList(const std::vector<Enemy*>& enemies) { enemies_ = enemies; }
    void SetInitialVelocity(const TuboEngine::Math::Vector3& vel) { SetVelocity(vel); }

private:
    Enemy* FindNearestEnemy();

    float elapsedTime_ = 0.0f;
    float speed_ = 1.0f;
    float turnSpeed_ = 0.12f;
    float swerveAmplitude_ = 0.8f;
    float swerveFrequency_ = 10.0f;
    float phase1Duration_ = 0.4f;

    TuboEngine::Math::Vector3 swerveOffset_{0, 0, 0};
    TuboEngine::Math::Vector3 lastTrailPos_{0, 0, 0}; // トレイルを隙間なく繋ぐための前回エミット位置
    
    class IParticleEmitter* trailEmitter_ = nullptr;
    class IParticleEmitter* explosionEmitter_ = nullptr;
    class IParticleEmitter* burnerEmitter_ = nullptr;
    class IParticleEmitter* sparkEmitter_ = nullptr;

    std::vector<Enemy*> enemies_;
    Enemy* currentTarget_ = nullptr;

    std::mt19937 rng_{std::random_device{}()};
};
