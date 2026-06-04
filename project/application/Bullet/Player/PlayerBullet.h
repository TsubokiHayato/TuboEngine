#pragma once
#include "Bullet/BaseBullet.h"
#include "MapChip/MapChipField.h"

class Player;

class PlayerBullet : public BaseBullet {
public:
    void Initialize(const TuboEngine::Math::Vector3& startPos) override;
    void Update() override;
    void Draw()   override;

    void OnCollision(Collider* other) override;
    TuboEngine::Math::Vector3 GetCenterPosition() const override;

    static void DrawImGuiGlobal();

    void SetPlayerPosition(const TuboEngine::Math::Vector3& p) { playerPosition_ = p; }
    void SetPlayerRotation(const TuboEngine::Math::Vector3& r) { playerRotation_  = r; }
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
