#pragma once
#include "Bullet/BaseBullet.h"
#include "MapChip/MapChipField.h"

class Player;

/// <summary>
/// 敵が発射する通常弾。直進し、マップチップとの衝突や射程距離で消滅する。
/// </summary>
class EnemyNormalBullet : public BaseBullet {
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
    void Draw()   override;

    /// <summary>
    /// 衝突時の処理。
    /// </summary>
    void OnCollision(Collider* other) override;
    /// <summary>
    /// 中心座標を取得する。
    /// </summary>
    TuboEngine::Math::Vector3 GetCenterPosition() const override;

    /// <summary>
    /// 全インスタンス共有パラメータのImGui表示。
    /// </summary>
    static void DrawImGuiGlobal();

    /// <summary>
    /// マップチップフィールドの参照を設定する。
    /// </summary>
    void SetMapChipField(MapChipField* field) { mapChipField_ = field; }
    /// <summary>
    /// プレイヤーの参照を設定する。
    /// </summary>
    void SetPlayer(Player* player) { player_ = player; }
    /// <summary>
    /// EnemyPosition を設定する。
    /// </summary>
    void SetEnemyPosition(const TuboEngine::Math::Vector3& p) { enemyPosition_ = p; }
    /// <summary>
    /// EnemyRotation を設定する。
    /// </summary>
    void SetEnemyRotation(const TuboEngine::Math::Vector3& r) { enemyRotation_ = r; }

    // ---- 静的パラメータ (ImGui から調整) ----
    static float                      s_disappearRadius;
    static float                      s_bulletSpeed;
    static float                      s_disappearZ;
    static TuboEngine::Math::Vector3  s_scale;
    static TuboEngine::Math::Vector3  s_rotation;
    static int                        s_damage;
    static float                      s_fireInterval;

private:
    Player*                   player_       = nullptr;
    TuboEngine::Math::Vector3 enemyPosition_;
    TuboEngine::Math::Vector3 enemyRotation_;
    MapChipField*             mapChipField_ = nullptr;
};
