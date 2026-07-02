#pragma once
#include "Enemy.h"
#include "Bullet/Enemy/EnemyMissileBullet.h"

/// <summary>
/// ミサイル（EnemyMissileBullet）を上空から降らせる砲撃タイプの敵。
/// </summary>
class MortarEnemy : public Enemy {
public:
    /// <summary>
    /// コンストラクタ。
    /// </summary>
    MortarEnemy() = default;
    /// <summary>
    /// デストラクタ。
    /// </summary>
    ~MortarEnemy() override = default;

    /// <summary>
    /// 初期化処理。
    /// </summary>
    void Initialize() override;
    /// <summary>
    /// 更新処理。
    /// </summary>
    void Update() override;
    /// <summary>
    /// 描画処理。
    /// </summary>
    void Draw() override;
    /// <summary>
    /// ImGuiによるデバッグ表示。
    /// </summary>
    void DrawImGui();

    /// <summary>
    /// ビヘイビアツリーの構築。
    /// </summary>
    void BuildBehaviorTree() override;
    /// <summary>
    /// モデルの色を取得する。
    /// </summary>
    TuboEngine::Math::Vector4 GetModelColor() const override { return {0.8f, 0.2f, 1.0f, 1.0f}; }

    /// <summary>
    /// MissileInterval を設定する。
    /// </summary>
    void SetMissileInterval(float sec) { missileInterval_ = sec; }
    /// <summary>
    /// MissileSpawnHeight を設定する。
    /// </summary>
    void SetMissileSpawnHeight(float height) { missileSpawnHeight_ = height; }
    /// <summary>
    /// MissileImpactRadius を設定する。
    /// </summary>
    void SetMissileImpactRadius(float radius) { missileImpactRadius_ = radius; }

private:
    /// <summary>
    /// ミサイル発射を試みる。
    /// </summary>
    void TryFireMissile(bool canSeePlayer, float dt);
    /// <summary>
    /// 砲台部分の変換行列を更新する。
    /// </summary>
    void UpdateArtilleryTransform();

private:
    std::unique_ptr<EnemyMissileBullet> missile_;
    float missileTimer_ = 0.0f;
    float missileInterval_ = 2.5f;
    float missileSpawnHeight_ = 15.0f;
    float missileImpactRadius_ = 2.0f;

    // 砲台シルエット用モデル（頭の上に配置）
    std::unique_ptr<TuboEngine::Object3d> artilleryObject_;

    // ImGui から調整するためのオフセット・スケール係数
    TuboEngine::Math::Vector3 artilleryOffset_ = {0.0f, 0.0f, 1.0f};
    float artilleryScaleFactor_ = 0.8f;

    // 発射エフェクト用
    IParticleEmitter* fireEmitter_ = nullptr;
};

