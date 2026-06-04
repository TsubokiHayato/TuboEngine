#pragma once
#include "Bullet/BaseBullet.h"
#include "MapChip/MapChipField.h"

class Player;

class EnemyNormalBullet : public BaseBullet {
public:
    void Initialize(const TuboEngine::Math::Vector3& startPos) override;
    void Update() override;
    void Draw()   override;

    void OnCollision(Collider* other) override;
    TuboEngine::Math::Vector3 GetCenterPosition() const override;

    static void DrawImGuiGlobal();

    void SetMapChipField(MapChipField* field) { mapChipField_ = field; }
    void SetPlayer(Player* player) { player_ = player; }
    void SetEnemyPosition(const TuboEngine::Math::Vector3& p) { enemyPosition_ = p; }
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
