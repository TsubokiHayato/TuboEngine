#pragma once
#include "Bullet/BaseBullet.h"
#include "Object3d.h"
#include "Vector3.h"

class Player;

/// <summary>
/// 敵（MortarEnemy）が発射するミサイル弾。目標地点へ上空から落下し、着弾範囲を持つ。
/// </summary>
class EnemyMissileBullet : public BaseBullet {
public:
    /// <summary>
    /// 初期化処理。
    /// </summary>
    void Initialize(const TuboEngine::Math::Vector3& startPos) override;
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
    /// <summary>
    /// 中心座標を取得する。
    /// </summary>
    TuboEngine::Math::Vector3 GetCenterPosition() const override;

    /// <summary>
    /// プレイヤーの参照を設定する。
    /// </summary>
    void SetPlayer(Player* player) { player_ = player; }
    /// <summary>
    /// TargetPosition を設定する。
    /// </summary>
    void SetTargetPosition(const TuboEngine::Math::Vector3& pos) { targetPosition_ = pos; }
    /// <summary>
    /// ImpactRadius を設定する。
    /// </summary>
    void SetImpactRadius(float radius) { impactRadius_ = radius; }
    /// <summary>
    /// カメラを設定する。
    /// </summary>
    void SetCamera(TuboEngine::Camera* camera);

    /// <summary>
    /// 生存フラグを取得する。
    /// </summary>
    bool GetIsAlive() const { return isAlive; }

public:
    static float s_fallSpeed;
    static float s_impactRadius;
    static TuboEngine::Math::Vector3 s_scale;
    static TuboEngine::Math::Vector3 s_rotation;

private:
    Player* player_ = nullptr;
    TuboEngine::Math::Vector3 targetPosition_{0.0f, 0.0f, 0.0f};
    TuboEngine::Math::Vector3 startPosition_{0.0f, 0.0f, 0.0f};
    float impactRadius_ = 2.0f;
    float fallSpeed_ = 0.0f;
    float gravity_ = 0.0f;
    float flightTime_ = 0.0f;
    float elapsedTime_ = 0.0f;
    TuboEngine::Camera* camera_ = nullptr;

    std::unique_ptr<TuboEngine::Object3d> targetObject_; // 着弾地点表示用

    class IParticleEmitter* impactEmitter_ = nullptr; // 着弾時の衝撃波
};


