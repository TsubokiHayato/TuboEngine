#pragma once
#include "Object3d.h"
#include "Object3dCommon.h"
#include "Vector3.h"
#include "engine/Collider/Collider.h"

class BaseBullet : public Collider {
public:
    virtual ~BaseBullet() = default;

    virtual void Initialize(const TuboEngine::Math::Vector3& startPos) = 0;
    virtual void Update() = 0;
    virtual void Draw()   = 0;

    virtual void OnCollision(Collider* other) override;
    virtual TuboEngine::Math::Vector3 GetCenterPosition() const override;

    // ---- 共通アクセサ ----
    bool GetIsAlive() const { return isAlive; }
    void SetIsAlive(bool alive) { isAlive = alive; }

    const TuboEngine::Math::Vector3& GetPosition() const { return position; }
    void SetPosition(const TuboEngine::Math::Vector3& p) { position = p; }

    const TuboEngine::Math::Vector3& GetRotation() const { return rotation; }
    void SetRotation(const TuboEngine::Math::Vector3& r) { rotation = r; }

    const TuboEngine::Math::Vector3& GetScale() const { return scale; }
    void SetScale(const TuboEngine::Math::Vector3& s) { scale = s; }

    const TuboEngine::Math::Vector3& GetVelocity() const { return velocity; }
    void SetVelocity(const TuboEngine::Math::Vector3& v) { velocity = v; }

    void SetCamera(TuboEngine::Camera* camera) { if (object3d) object3d->SetCamera(camera); }

protected:
    TuboEngine::Math::Vector3 position;
    TuboEngine::Math::Vector3 velocity;
    TuboEngine::Math::Vector3 rotation;
    TuboEngine::Math::Vector3 scale;

    bool  isAlive     = false;
    bool  isHit       = false;
    float bulletSpeed = 0.0f;
    float disappearZ  = 100.0f;

    int reflectCount    = 0;
    int maxReflectCount = 2;

    std::unique_ptr<TuboEngine::Object3d> object3d;

    // position/rotation/scale を object3d に反映して Update() する共通処理
    void ApplyTransformToObject3d();
};
