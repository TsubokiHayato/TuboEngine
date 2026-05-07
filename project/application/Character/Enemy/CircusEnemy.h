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
    void DrawSprite();
    void DrawImGui();

private:
    void TryFireMissiles(bool canSeePlayer, float dt);
    void FireSingleMissile(const TuboEngine::Math::Vector3& launchDir, float speed);

    bool isCharging_ = false;
    class IParticleEmitter* chargeEmitter_ = nullptr;
    class IParticleEmitter* muzzleFlashEmitter_ = nullptr;
    class IParticleEmitter* auraEmitter_ = nullptr;
    class IParticleEmitter* explosionEmitter_ = nullptr; // 死亡時の連鎖大爆発用

    // 死亡演出用
    float deathTimer_ = 0.0f;
    float nextExplosionTime_ = 0.0f;
    bool isDying_ = false; 

    std::unique_ptr<TuboEngine::Sprite> bossHpFrameSprite_;
    std::unique_ptr<TuboEngine::Sprite> bossHpBarSprite_;
    std::unique_ptr<TuboEngine::Sprite> bossHpBarBgSprite_; // 遅延用の背景バー

    // HPのUIパーティクル
    struct SimpleUIParticle {
        std::unique_ptr<TuboEngine::Sprite> sprite;
        TuboEngine::Math::Vector2 position;
        TuboEngine::Math::Vector2 velocity;
        float life = 0.0f;
        float maxLife = 1.0f;
        TuboEngine::Math::Vector4 color;
        bool active = false;
    };
    std::vector<SimpleUIParticle> hpParticles_;
    void EmitHpParticle(const TuboEngine::Math::Vector2& pos);
    void UpdateHpParticles(float dt);
    void DrawHpParticles();

    float animatedHp_ = 350.0f; // Initialize()で上書きされるが念のため
    float hpParticleTimer_ = 0.0f;

    // ImGuiでの微調整用パラメータ
    TuboEngine::Math::Vector2 hpFrameOffset_ = {0.0f, 0.0f};
    TuboEngine::Math::Vector2 hpBarOffset_ = {0.0f, 0.0f};
    TuboEngine::Math::Vector2 hpBarSizeBase_ = {600.0f, 32.0f};
    TuboEngine::Math::Vector2 hpFrameSizeBase_ = {600.0f, 32.0f};

private:
    enum class AttackType {
        Burst,      // 一斉射撃
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
    int missileCount_ = 24;
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
    float bulletPhase1Duration_ = 0.4f;
    int bulletTargetDelayFrames_ = 25; // ターゲットの遅延フレーム数

    int maxHp_ = 100; // ボスの最大HP

    // スパイラル攻撃用
    float spiralAngle_ = 0.0f;

    // バースト連射用メンバ
    int remainingMissiles_ = 0;
    float burstTimer_ = 0.0f;
    const float kBurstDelay = 0.03f; // 1発ごとの間隔
};
