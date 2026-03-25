#include "MortarEnemy.h"
#include "Character/Player/Player.h"
#include <algorithm>
#include <cmath>

namespace {
constexpr float kPI = 3.14159265358979323846f;
constexpr float kMortarEnemyModelRotOffsetX = 0.0f;
constexpr float kMortarEnemyModelRotOffsetY = 0.0f;
constexpr float kMortarEnemyModelRotOffsetZ = -kPI * 0.5f;

float NormalizeAngle(float angle) {
    while (angle > kPI) {
        angle -= 2.0f * kPI;
    }
    while (angle < -kPI) {
        angle += 2.0f * kPI;
    }
    return angle;
}
}

void MortarEnemy::Initialize() {
    Enemy::Initialize();
    moveSpeed_ = 0.0f;
    shootDistance_ = 0.0f;
    missileTimer_ = 0.0f;
    missileInterval_ = 1.2f;
    missileSpawnHeight_ = 15.0f;
    missileImpactRadius_ = EnemyMissileBullet::s_impactRadius;
}

void MortarEnemy::Update() {
    if (!isAlive) {
        if (!deathEffectPlayed_) {
            EmitDeathParticle();
            deathEffectPlayed_ = true;
        }
        if (deathEmitter_) {
            deathEmitter_->GetPreset().center = position;
        }
        if (missile_ && missile_->GetIsAlive()) {
            missile_->Update();
        } else if (missile_ && !missile_->GetIsAlive()) {
            missile_.reset();
        }
        return;
    }

    const float dt = 1.0f / 60.0f;

    ApplyKnockback(dt);

    bool canSeePlayer = CanSeePlayer();
    wasJustFound_ = (!sawPlayerPrev_ && canSeePlayer);
    wasJustLost_ = (sawPlayerPrev_ && !canSeePlayer);
    sawPlayerPrev_ = canSeePlayer;

    if (canSeePlayer) {
        lastSeenPlayerPos = player_->GetPosition();
        lastSeenTimer = kLastSeenDuration;
        if (wasJustFound_) {
            exclamationTimer_ = iconDuration_;
            questionTimer_ = 0.0f;
        }
    } else if (lastSeenTimer > 0.0f) {
        lastSeenTimer -= dt;
        if (wasJustLost_) {
            questionTimer_ = iconDuration_;
            exclamationTimer_ = 0.0f;
        }
    }

    if (questionTimer_ > 0.0f) {
        questionTimer_ -= dt;
        if (questionTimer_ < 0.0f) {
            questionTimer_ = 0.0f;
        }
    }
    if (exclamationTimer_ > 0.0f) {
        exclamationTimer_ -= dt;
        if (exclamationTimer_ < 0.0f) {
            exclamationTimer_ = 0.0f;
        }
    }

    if (player_) {
        if (canSeePlayer) {
            state_ = State::Attack;
        } else if (lastSeenTimer > 0.0f) {
            state_ = State::Alert;
        } else if (state_ == State::Alert) {
            state_ = State::LookAround;
        } else if (state_ != State::LookAround) {
            state_ = State::Idle;
        }
    }

    if (player_ && (state_ == State::Attack || state_ == State::Alert)) {
        TuboEngine::Math::Vector3 targetPos = (canSeePlayer) ? player_->GetPosition() : lastSeenPlayerPos;
        TuboEngine::Math::Vector3 toTarget = targetPos - position;
        float angleZ = std::atan2(toTarget.y, toTarget.x);
        float diff = NormalizeAngle(angleZ - rotation.z);
        float maxTurn = turnSpeed_;
        if (std::fabs(diff) < maxTurn) {
            rotation.z = angleZ;
        } else {
            rotation.z += (diff > 0 ? 1 : -1) * maxTurn;
            rotation.z = NormalizeAngle(rotation.z);
        }
    }

    TryFireMissile(canSeePlayer, dt);

    if (missile_ && missile_->GetIsAlive()) {
        missile_->Update();
    } else if (missile_ && !missile_->GetIsAlive()) {
        missile_.reset();
    }

    object3d->SetPosition(position);
    TuboEngine::Math::Vector3 drawRot = rotation;
    drawRot.x = NormalizeAngle(drawRot.x + kMortarEnemyModelRotOffsetX);
    drawRot.y = NormalizeAngle(drawRot.y + kMortarEnemyModelRotOffsetY);
    drawRot.z = NormalizeAngle(drawRot.z + kMortarEnemyModelRotOffsetZ);
    object3d->SetRotation(drawRot);
    object3d->SetScale(scale);
    object3d->SetCamera(camera_);
    object3d->Update();

    if (hitEmitter_) {
        hitEmitter_->GetPreset().center = position;
    }
    if (deathEmitter_ && !deathEffectPlayed_) {
        deathEmitter_->GetPreset().center = position;
    }

    if (!wasHit && isHit) {
        EmitHitParticle();
    }
    wasHit = isHit;
    isHit = false;

    TuboEngine::Math::Vector3 iconWorldPos = position;
    iconWorldPos.z = position.z;
    iconWorldPos.y += iconOffsetY_;
    if (questionTimer_ > 0.0f && questionIcon_) {
        questionIcon_->SetPosition({iconWorldPos.x, iconWorldPos.y});
        questionIcon_->SetSize(iconSize_);
        questionIcon_->Update();
    }
    if (exclamationTimer_ > 0.0f && exclamationIcon_) {
        exclamationIcon_->SetPosition({iconWorldPos.x, iconWorldPos.y});
        exclamationIcon_->SetSize(iconSize_);
        exclamationIcon_->Update();
    }
}

void MortarEnemy::TryFireMissile(bool canSeePlayer, float dt) {
    if (!player_) {
        return;
    }

    if (canSeePlayer) {
        missileTimer_ += dt;
    } else {
        missileTimer_ = std::min(missileTimer_, missileInterval_);
        return;
    }

    if (missile_ && missile_->GetIsAlive()) {
        return;
    }

    if (missileTimer_ < missileInterval_) {
        return;
    }

    missileTimer_ = 0.0f;

    TuboEngine::Math::Vector3 targetPos = player_->GetCenterPosition();
    TuboEngine::Math::Vector3 startPos = position;

    missile_ = std::make_unique<EnemyMissileBullet>();
    missile_->SetTargetPosition(targetPos);
    missile_->SetImpactRadius(missileImpactRadius_);
    missile_->SetPlayer(player_);
    missile_->SetCamera(camera_);
    missile_->Initialize(startPos);
}

void MortarEnemy::Draw() {
    if (object3d && isAlive) {
        object3d->Draw();
    }
    if (missile_) {
        missile_->Draw();
    }
    DrawViewCone();
    DrawLastSeenMark();
    DrawStateIcon();
}
