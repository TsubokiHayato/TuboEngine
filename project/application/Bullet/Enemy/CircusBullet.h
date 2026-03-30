#pragma once
#include "Bullet/BaseBullet.h"
#include "Object3d.h"
#include "Vector3.h"

class Player;

class CircusBullet : public BaseBullet {
public:
    void Initialize(const TuboEngine::Math::Vector3& startPos) override;
    void Update() override;
    void Draw() override;
    void OnCollision(Collider* other) override;
    TuboEngine::Math::Vector3 GetCenterPosition() const override;

    void SetPlayer(Player* player) { player_ = player; }
    void SetCamera(TuboEngine::Camera* camera);
    void SetInitialVelocity(const TuboEngine::Math::Vector3& vel) { velocity = vel; }
    void SetTargetOffset(const TuboEngine::Math::Vector3& offset) { targetOffset_ = offset; }

    bool GetIsAlive() const { return isAlive; }

private:
    Player* player_ = nullptr;
    TuboEngine::Camera* camera_ = nullptr;
    
    float speed_ = 1.0f;
    float turnSpeed_ = 0.08f;
    float chaosAmplitude_ = 0.4f;
    float chaosFrequency_ = 5.0f;
    float phase1Duration_ = 0.5f;
    float elapsedTime_ = 0.0f;

    TuboEngine::Math::Vector3 swerveOffset_{0, 0, 0};
    TuboEngine::Math::Vector3 targetOffset_{0, 0, 0};
    class IParticleEmitter* trailEmitter_ = nullptr;
};
