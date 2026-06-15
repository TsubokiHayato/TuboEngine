#include "EnemyNormalBullet.h"
#include "Character/Player/Player.h"
#include "Collider/CollisionTypeId.h"
#include "TextureManager.h"
#include "ImGuiManager.h"
#include <cmath>

namespace {
constexpr float                     kDefaultBulletSpeed    = 0.5f;
constexpr TuboEngine::Math::Vector3 kDefaultBulletScale    = {1.0f, 1.0f, 1.0f};
constexpr TuboEngine::Math::Vector3 kDefaultBulletRotation = {0.0f, 0.0f, 0.0f};
constexpr float                     kBulletDisappearZ      = 100.0f;
}

float                     EnemyNormalBullet::s_bulletSpeed     = kDefaultBulletSpeed;
float                     EnemyNormalBullet::s_disappearZ      = kBulletDisappearZ;
TuboEngine::Math::Vector3 EnemyNormalBullet::s_scale           = kDefaultBulletScale;
TuboEngine::Math::Vector3 EnemyNormalBullet::s_rotation        = kDefaultBulletRotation;
float                     EnemyNormalBullet::s_disappearRadius = 50.0f;
int                       EnemyNormalBullet::s_damage          = 1;
float                     EnemyNormalBullet::s_fireInterval    = 1.0f;

void EnemyNormalBullet::Initialize(const TuboEngine::Math::Vector3& startPos) {
    Collider::SetTypeID(static_cast<uint32_t>(CollisionTypeId::kEnemyWeapon));
    position    = startPos;
    velocity    = {};
    isAlive     = true;
    bulletSpeed = s_bulletSpeed;
    disappearZ  = s_disappearZ;
    scale       = s_scale;
    rotation    = s_rotation;

    Collider::SetRadius(0.5f);

    object3d = std::make_unique<TuboEngine::Object3d>();
    object3d->Initialize("star.obj");
    object3d->SetLightType(5);
    object3d->SetPosition(position);
    object3d->SetRotation(rotation);
    object3d->SetScale(scale);
}

void EnemyNormalBullet::Update() {
    if (!isAlive) return;

    isHit       = false;
    bulletSpeed = s_bulletSpeed;
    scale       = s_scale;
    rotation    = s_rotation;

    float angle = enemyRotation_.z - DirectX::XM_PIDIV2;
    velocity.x  = -std::sinf(angle) * bulletSpeed;
    velocity.y  =  std::cosf(angle) * bulletSpeed;
    velocity.z  = 0.0f;

    TuboEngine::Math::Vector3 desiredMove = velocity;

    float tileSize  = (mapChipField_) ? MapChipField::GetBlockSize() : 1.0f;
    float moveLen2D = std::sqrt(desiredMove.x * desiredMove.x + desiredMove.y * desiredMove.y);
    int subSteps    = std::max(1, int(std::ceil(moveLen2D / (tileSize * 0.5f))));
    TuboEngine::Math::Vector3 stepMove = desiredMove / float(subSteps);

    for (int i = 0; i < subSteps && isAlive; ++i) {
        TuboEngine::Math::Vector3 nextPos = position + stepMove;

        if (mapChipField_ && mapChipField_->IsBlocked(nextPos)) {
            isAlive = false;
            break;
        }

        position = nextPos;

        if (player_) {
            constexpr float bulletR = 0.5f;
            constexpr float playerR = 1.0f;
            TuboEngine::Math::Vector3 pc = player_->GetCenterPosition();
            float dx   = position.x - pc.x;
            float dy   = position.y - pc.y;
            float rSum = bulletR + playerR;
            if (dx*dx + dy*dy <= rSum*rSum) {
                player_->OnCollision(this);
                isHit   = true;
                isAlive = false;
                break;
            }
        }
    }

    if (isAlive && player_) {
        TuboEngine::Math::Vector3 p = player_->GetPosition();
        float dx   = position.x - p.x;
        float dy   = position.y - p.y;
        if (dx*dx + dy*dy > s_disappearRadius * s_disappearRadius) isAlive = false;
    }

    if (isAlive && position.z > s_disappearZ) isAlive = false;

    if (!isAlive) return;
    ApplyTransformToObject3d();
}

void EnemyNormalBullet::Draw() {
    if (isAlive) object3d->Draw();
}

TuboEngine::Math::Vector3 EnemyNormalBullet::GetCenterPosition() const { return position; }

void EnemyNormalBullet::OnCollision(Collider* other) {
    if (other->GetTypeID() == static_cast<uint32_t>(CollisionTypeId::kPlayer)) {
        isHit   = true;
        isAlive = false;
    }
}

void EnemyNormalBullet::DrawImGuiGlobal() {
#ifdef USE_IMGUI
    ImGui::Begin("Enemy Normal Bullet");
    ImGui::DragFloat("Speed",            &s_bulletSpeed,     0.01f, 0.0f,   10.0f);
    ImGui::DragFloat("Disappear Radius", &s_disappearRadius, 0.1f,  0.0f,  200.0f);
    ImGui::DragFloat3("Scale",           &s_scale.x,         0.01f);
    ImGui::DragFloat3("Rotation",        &s_rotation.x,      0.01f);
    ImGui::End();
#endif
}
