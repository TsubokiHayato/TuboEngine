#pragma once
#include "Bullet/Player/PlayerBullet.h"
#include <vector>
#include <random>

class Enemy;

/// <summary>
/// ジャスト回避成功時のカウンター用弾。敵のサーカス弾と同等のホーミングと演出を持つ。
/// </summary>
class PlayerCircusBullet : public PlayerBullet {
public:
    /// <summary>
    /// 初期化処理。
    /// </summary>
    void Initialize(const TuboEngine::Math::Vector3& startPos) override;
    /// <summary>
    /// サーカス弾固有の初期化処理。
    /// </summary>
    void InitializeCircus(const TuboEngine::Math::Vector3& startPos, TuboEngine::Camera* camera, class CircusEnemy* boss = nullptr);
    /// <summary>
    /// 更新処理。
    /// </summary>
    void Update() override;
    /// <summary>
    /// 描画処理。
    /// </summary>
    void Draw() override;
    /// <summary>
    /// 衝突時の処理。
    /// </summary>
    void OnCollision(Collider* other) override;

    // カウンター特有の設定
    void SetTargetEnemyList(const std::vector<Enemy*>& enemies) { enemies_ = enemies; }
    void SetInitialVelocity(const TuboEngine::Math::Vector3& vel) { SetVelocity(vel); }

private:
    /// <summary>
    /// 最も近い敵を探して返す。
    /// </summary>
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
