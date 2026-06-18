#pragma once
#include "Bullet/BaseBullet.h"
#include "Object3d.h"
#include "Vector3.h"
#include <random>

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
    void SetSpeed(float speed) { speed_ = speed; }
    void SetTurnSpeed(float turnSpeed) { turnSpeed_ = turnSpeed; }
    void SetSwerveAmplitude(float amp) { swerveAmplitude_ = amp; }
    void SetSwerveFrequency(float freq) { swerveFrequency_ = freq; }
    void SetPhase1Duration(float duration) { phase1Duration_ = duration; }
    void SetTargetDelayFrames(int delayFrames) { targetDelayFrames_ = delayFrames; }

    bool GetIsAlive() const { return isAlive; }
    void SetIsAlive(bool alive) { isAlive = alive; }
    TuboEngine::Math::Vector3 GetPosition() const { return position; }

private:
    Player* player_ = nullptr;
    TuboEngine::Camera* camera_ = nullptr;
    
    float speed_ = 0.6f;
    float turnSpeed_ = 0.08f;
    float swerveAmplitude_ = 0.4f;
    float swerveFrequency_ = 5.0f;
    float phase1Duration_ = 0.5f;
    float elapsedTime_ = 0.0f;
    int targetDelayFrames_ = 25;

    TuboEngine::Math::Vector3 swerveOffset_{0, 0, 0};
    TuboEngine::Math::Vector3 targetOffset_{0, 0, 0};
    TuboEngine::Math::Vector3 lastTrailPos_{0, 0, 0}; // トレイルを隙間なく繋ぐための前回エミット位置
    class IParticleEmitter* trailEmitter_ = nullptr;
    class IParticleEmitter* explosionEmitter_ = nullptr;
    class IParticleEmitter* burnerEmitter_ = nullptr;
    class IParticleEmitter* sparkEmitter_ = nullptr; // 火花エミッター

    std::mt19937 rng_{std::random_device{}()};
};
