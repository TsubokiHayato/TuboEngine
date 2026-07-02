#pragma once
#include "Bullet/BaseBullet.h"
#include "MapChip/MapChipField.h"

class Player;

/// <summary>
/// プレイヤーが発射する弾。直進し、マップチップとの衝突や射程距離で消滅する。
/// </summary>
class PlayerBullet : public BaseBullet {
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
    /// PlayerPosition を設定する。
    /// </summary>
    void SetPlayerPosition(const TuboEngine::Math::Vector3& p) { playerPosition_ = p; }
    /// <summary>
    /// PlayerRotation を設定する。
    /// </summary>
    void SetPlayerRotation(const TuboEngine::Math::Vector3& r) { playerRotation_  = r; }
    /// <summary>
    /// マップチップフィールドの参照を設定する。
    /// </summary>
    void SetMapChipField(MapChipField* field) { mapChipField_ = field; }

    // ---- 静的パラメータ (ImGui から調整) ----
    static float                      s_disappearRadius;
    static float                      s_bulletSpeed;
    static float                      s_disappearZ;
    static TuboEngine::Math::Vector3  s_scale;
    static TuboEngine::Math::Vector3  s_rotation;
    static int                        s_damage;
    static float                      s_fireInterval;

private:
    TuboEngine::Math::Vector3 playerPosition_;
    TuboEngine::Math::Vector3 playerRotation_;
    MapChipField*             mapChipField_ = nullptr;
};
