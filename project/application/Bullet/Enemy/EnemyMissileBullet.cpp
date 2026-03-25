#include "EnemyMissileBullet.h"
#include "Character/Player/Player.h"
#include "Collider/CollisionTypeId.h"
#include "LineManager.h"
#include <cmath>
#include <algorithm>

namespace {
constexpr float kDefaultFallSpeed = 40.0f;
constexpr float kDefaultImpactRadius = 2.0f;
constexpr TuboEngine::Math::Vector3 kDefaultScale = {1.0f, 1.0f, 1.0f};
constexpr TuboEngine::Math::Vector3 kDefaultRotation = {0.0f, 0.0f, 0.0f};
constexpr float kFixedDeltaTime = 1.0f / 60.0f;
constexpr float kMinApexHeight = 14.0f;
constexpr float kExtraApexHeight = 10.0f;
}

float EnemyMissileBullet::s_fallSpeed = kDefaultFallSpeed;
float EnemyMissileBullet::s_impactRadius = kDefaultImpactRadius;
TuboEngine::Math::Vector3 EnemyMissileBullet::s_scale = kDefaultScale;
TuboEngine::Math::Vector3 EnemyMissileBullet::s_rotation = kDefaultRotation;

void EnemyMissileBullet::Initialize(const TuboEngine::Math::Vector3& startPos) {
    Collider::SetTypeID(static_cast<uint32_t>(CollisionTypeId::kEnemyWeapon));
    position = startPos;
    startPosition_ = startPos;
    velocity = {0.0f, 0.0f, 0.0f};
    rotation = s_rotation;
    scale = s_scale;
    fallSpeed_ = s_fallSpeed;
    gravity_ = std::max(0.01f, s_fallSpeed);
    impactRadius_ = s_impactRadius;
    isAlive = true;
    elapsedTime_ = 0.0f;

    const TuboEngine::Math::Vector3 delta = targetPosition_ - startPos;
    const float distance2D = std::sqrt(delta.x * delta.x + delta.y * delta.y);
    const float baseApex = std::max(kMinApexHeight, std::abs(delta.z) + kExtraApexHeight);
    const float apexHeight = baseApex;
    const float timeUp = std::sqrt(std::max(0.0f, 2.0f * apexHeight / gravity_));
    const float timeDown = std::sqrt(std::max(0.0f, 2.0f * (apexHeight + delta.z) / gravity_));
    flightTime_ = std::max(0.1f, timeUp + timeDown);
    velocity.x = (flightTime_ > 0.0f) ? (delta.x / flightTime_) : 0.0f;
    velocity.y = (flightTime_ > 0.0f) ? (delta.y / flightTime_) : 0.0f;
    velocity.z = gravity_ * timeUp;

    Collider::SetRadius(impactRadius_);

    object3d = std::make_unique<TuboEngine::Object3d>();
    object3d->Initialize("block/block.obj");
    object3d->SetPosition(position);
    object3d->SetRotation(rotation);
    object3d->SetScale(scale);
    if (camera_) {
        object3d->SetCamera(camera_);
    }
}

void EnemyMissileBullet::SetCamera(TuboEngine::Camera* camera) {
    camera_ = camera;
    if (object3d) {
        object3d->SetCamera(camera);
    }
}

void EnemyMissileBullet::Update() {
    if (!isAlive) {
        return;
    }

    fallSpeed_ = s_fallSpeed;
    gravity_ = std::max(0.01f, s_fallSpeed);
    scale = s_scale;
    rotation = s_rotation;

    const float dt = kFixedDeltaTime;
    velocity.z -= gravity_ * dt;
    position = position + velocity * dt;
    elapsedTime_ += dt;

    if (player_) {
        TuboEngine::Math::Vector3 pc = player_->GetCenterPosition();
        float dx = position.x - pc.x;
        float dy = position.y - pc.y;
        float dz = position.z - pc.z;
        float r = impactRadius_;
        if (dx * dx + dy * dy + dz * dz <= r * r) {
            player_->OnCollision(this);
            isAlive = false;
        }
    }

    if (isAlive && (position.z <= targetPosition_.z || elapsedTime_ >= flightTime_)) {
        if (player_) {
            TuboEngine::Math::Vector3 pc = player_->GetCenterPosition();
            float dx = pc.x - targetPosition_.x;
            float dy = pc.y - targetPosition_.y;
            float r = impactRadius_;
            if (dx * dx + dy * dy <= r * r) {
                player_->OnCollision(this);
            }
        }
        isAlive = false;
    }

    if (!isAlive) {
        return;
    }

    object3d->SetPosition(position);
    object3d->SetRotation(rotation);
    object3d->SetScale(scale);
    object3d->Update();
}

void EnemyMissileBullet::Draw() {
    if (isAlive && object3d) {
        object3d->Draw();
    }
    if (isAlive) {
        TuboEngine::LineManager::GetInstance()->DrawLine(startPosition_, targetPosition_, {1.0f, 0.3f, 0.2f, 0.6f});
    }
}

void EnemyMissileBullet::OnCollision(Collider* other) {
    if (!other) {
        return;
    }
    if (other->GetTypeID() == static_cast<uint32_t>(CollisionTypeId::kPlayer)) {
        isAlive = false;
    }
}

TuboEngine::Math::Vector3 EnemyMissileBullet::GetCenterPosition() const {
    return position;
}
