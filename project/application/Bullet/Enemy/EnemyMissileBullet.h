#pragma once
#include "Bullet/BaseBullet.h"
#include "Object3d.h"
#include "Vector3.h"

class Player;

class EnemyMissileBullet : public BaseBullet {
public:
    void Initialize(const TuboEngine::Math::Vector3& startPos) override;
    void Update() override;
    void Draw() override;
    void OnCollision(Collider* other) override;
    TuboEngine::Math::Vector3 GetCenterPosition() const override;

    void SetPlayer(Player* player) { player_ = player; }
    void SetTargetPosition(const TuboEngine::Math::Vector3& pos) { targetPosition_ = pos; }
    void SetImpactRadius(float radius) { impactRadius_ = radius; }
    void SetCamera(TuboEngine::Camera* camera);

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


