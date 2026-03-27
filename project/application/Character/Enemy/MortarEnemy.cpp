#include "MortarEnemy.h"
#include "Character/Player/Player.h"
#include"ImguiManager.h"
#include <algorithm>
#include <cmath>
#include "engine/graphic/Particle/Effects/Primitive/PrimitiveEmitter.h"


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

    // 砲台シルエットモデル初期化（頭の上に載せる）
    artilleryObject_ = std::make_unique<TuboEngine::Object3d>();
    artilleryObject_->Initialize("artillery battery/artillery battery.obj");
    if (camera_) {
        artilleryObject_->SetCamera(camera_);
    }
    artilleryObject_->SetLightType(5); // EnvironmentMap で少し目立たせる
    
    // 本体を紫に変更
    object3d->SetModelColor({0.8f, 0.2f, 1.0f, 1.0f});
    // 砲台も同系色に
    artilleryObject_->SetModelColor({0.6f, 0.1f, 0.8f, 1.0f});

    // --- 発射エフェクト用 ---
    if (!fireEmitter_) {
        ParticlePreset p{};
        p.name = "MortarFire";
        p.texture = "particle.png";
        p.maxInstances = 64;
        p.autoEmit = false;
        p.burstCount = 8;
        p.lifeMin = 0.2f;
        p.lifeMax = 0.4f;
        p.scaleStart = {0.3f, 0.3f, 0.3f};
        p.scaleEnd = {0.8f, 0.8f, 0.8f};
        p.colorStart = {1.0f, 0.8f, 0.4f, 1.0f};
        p.colorEnd = {0.5f, 0.5f, 0.5f, 0.0f};
        p.velMin = {-1.0f, -1.0f, 1.0f};
        p.velMax = {1.0f, 1.0f, 3.0f};
        p.gravity = {0, 0, 0};
        fireEmitter_ = TuboEngine::ParticleManager::GetInstance()->CreateEmitter<PrimitiveEmitter>(p);
    }
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

    ApplyHitShake(dt);


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

    object3d->SetPosition(position + hitShakeOffset_);

    TuboEngine::Math::Vector3 drawRot = rotation;
    drawRot.x = NormalizeAngle(drawRot.x + kMortarEnemyModelRotOffsetX);
    drawRot.y = NormalizeAngle(drawRot.y + kMortarEnemyModelRotOffsetY);
    drawRot.z = NormalizeAngle(drawRot.z + kMortarEnemyModelRotOffsetZ);
    object3d->SetRotation(drawRot);
    object3d->SetScale(scale);
    object3d->SetCamera(camera_);
    object3d->Update();

    // 砲台シルエット更新（頭の上に追従）
    UpdateArtilleryTransform();
    if (artilleryObject_) {
        artilleryObject_->SetCamera(camera_);
        artilleryObject_->Update();
    }

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

void MortarEnemy::UpdateArtilleryTransform() {
    if (!artilleryObject_) {
        return;
    }

    // 頭の少し上に配置（ImGui から調整可能）
    TuboEngine::Math::Vector3 artilleryPos = position + artilleryOffset_ + hitShakeOffset_;


    // MortarEnemy が向いている方向に合わせて砲台も回転（モデル軸補正を適用）
    TuboEngine::Math::Vector3 artilleryRot = rotation;
    artilleryRot.x = NormalizeAngle(artilleryRot.x + kMortarEnemyModelRotOffsetX);
    artilleryRot.y = NormalizeAngle(artilleryRot.y + kMortarEnemyModelRotOffsetY);
    artilleryRot.z = NormalizeAngle(artilleryRot.z + kMortarEnemyModelRotOffsetZ);

    // 本体スケールをベースに、係数で調整
    TuboEngine::Math::Vector3 artilleryScale = scale;
    artilleryScale.x *= artilleryScaleFactor_;
    artilleryScale.y *= artilleryScaleFactor_;
    artilleryScale.z *= artilleryScaleFactor_;

    artilleryObject_->SetPosition(artilleryPos);
    artilleryObject_->SetRotation(artilleryRot);
    artilleryObject_->SetScale(artilleryScale);
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

    // --- 演出：発射エフェクト ---
    if (fireEmitter_) {
        fireEmitter_->GetPreset().center = startPos + artilleryOffset_;
        fireEmitter_->Emit(fireEmitter_->GetPreset().burstCount);
    }
}


void MortarEnemy::Draw() {
    if (object3d && isAlive) {
        object3d->Draw();
    }
    if (artilleryObject_ && isAlive) {
        artilleryObject_->Draw();
    }
    if (missile_) {
        missile_->Draw();
    }
    DrawViewCone();
    DrawLastSeenMark();
    DrawStateIcon();
}

void MortarEnemy::DrawImGui() {
#ifdef USE_IMGUI
    ImGui::Begin("MortarEnemy");
    ImGui::Text("Pos:(%.2f,%.2f,%.2f)", position.x, position.y, position.z);
    ImGui::Text("HP:%d", HP);

    ImGui::DragFloat("MissileInterval", &missileInterval_, 0.05f, 0.1f, 10.0f);
    ImGui::DragFloat("MissileSpawnHeight", &missileSpawnHeight_, 0.1f, 0.0f, 50.0f);
    ImGui::DragFloat("MissileImpactRadius", &missileImpactRadius_, 0.05f, 0.1f, 10.0f);

    // 砲台シルエット用パラメータ
    ImGui::Separator();
    ImGui::Text("Artillery Silhouette");
    ImGui::DragFloat3("ArtilleryOffset (X,Y,Z)", &artilleryOffset_.x, 0.05f, -10.0f, 10.0f);
    ImGui::DragFloat("ArtilleryScaleFactor", &artilleryScaleFactor_, 0.01f, 0.1f, 3.0f);

    ImGui::End();
#endif
}
