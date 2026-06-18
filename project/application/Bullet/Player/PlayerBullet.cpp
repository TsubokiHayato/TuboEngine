#include "PlayerBullet.h"
#include "Collider/CollisionTypeId.h"
#include "ImGuiManager.h"
#include "Character/Player/Player.h"

namespace {
constexpr float                     kDefaultBulletSpeed    = 2.0f;
constexpr TuboEngine::Math::Vector3 kDefaultBulletScale    = {1.0f, 1.0f, 1.0f};
constexpr TuboEngine::Math::Vector3 kDefaultBulletRotation = {0.0f, 0.0f, 0.0f};
} // namespace

float                      PlayerBullet::s_disappearRadius = 100.0f;
float                      PlayerBullet::s_bulletSpeed     = kDefaultBulletSpeed;
float                      PlayerBullet::s_disappearZ      = 100.0f;
TuboEngine::Math::Vector3  PlayerBullet::s_scale           = kDefaultBulletScale;
TuboEngine::Math::Vector3  PlayerBullet::s_rotation        = kDefaultBulletRotation;
int                        PlayerBullet::s_damage          = 1;
float                      PlayerBullet::s_fireInterval    = 0.2f;

void PlayerBullet::Initialize(const TuboEngine::Math::Vector3& startPos) {
    Collider::SetTypeID(static_cast<uint32_t>(CollisionTypeId::kPlayerWeapon));
    position = startPos;

    bulletSpeed = s_bulletSpeed;
    velocity.x  =  std::sinf(playerRotation_.z) * -bulletSpeed;
    velocity.y  = -std::cosf(playerRotation_.z) * -bulletSpeed;
    velocity.z  = 0.0f;

    isAlive        = true;
    reflectCount   = 0;
    maxReflectCount = 2;

    disappearZ = s_disappearZ;
    scale      = s_scale;
    rotation   = s_rotation;

    object3d = std::make_unique<TuboEngine::Object3d>();
    object3d->Initialize("playerBullet/playerBullet.obj");
}

void PlayerBullet::Update() {
    if (!isAlive) return;

    isHit    = false;
    scale    = s_scale;
    rotation = s_rotation;

    TuboEngine::Math::Vector3 moveAmount = velocity;

    float bulletWidth  = 0.5f;
    float bulletHeight = 0.5f;

    float moveDist = std::sqrt(moveAmount.x * moveAmount.x + moveAmount.y * moveAmount.y);
    int subSteps = std::max(1, int(std::ceil(moveDist / 0.1f)));
    TuboEngine::Math::Vector3 stepDelta = moveAmount / (float)subSteps;

    for (int i = 0; i < subSteps; ++i) {
        TuboEngine::Math::Vector3 nextPosX = position;
        nextPosX.x += stepDelta.x;
        if (mapChipField_ && mapChipField_->IsRectBlocked(nextPosX, bulletWidth, bulletHeight)) {
            if (reflectCount < maxReflectCount) {
                velocity.x  *= -1.0f;
                stepDelta.x *= -1.0f;
                reflectCount++;
            } else {
                isAlive = false;
                break;
            }
        } else {
            position.x = nextPosX.x;
        }

        TuboEngine::Math::Vector3 nextPosY = position;
        nextPosY.y += stepDelta.y;
        if (mapChipField_ && mapChipField_->IsRectBlocked(nextPosY, bulletWidth, bulletHeight)) {
            if (reflectCount < maxReflectCount) {
                velocity.y  *= -1.0f;
                stepDelta.y *= -1.0f;
                reflectCount++;
            } else {
                isAlive = false;
                break;
            }
        } else {
            position.y = nextPosY.y;
        }

        if (!isAlive) break;
    }

    if (position.z > disappearZ) isAlive = false;

    if (isAlive) {
        float dx   = position.x - playerPosition_.x;
        float dy   = position.y - playerPosition_.y;
        float dz   = position.z - playerPosition_.z;
        float dist = std::sqrt(dx*dx + dy*dy + dz*dz);
        if (dist > s_disappearRadius) isAlive = false;
    }

    if (!isAlive) return;
    ApplyTransformToObject3d();
}

void PlayerBullet::Draw() { object3d->Draw(); }

TuboEngine::Math::Vector3 PlayerBullet::GetCenterPosition() const { return position; }

void PlayerBullet::OnCollision(Collider* other) {
    if (other->GetTypeID() == static_cast<uint32_t>(CollisionTypeId::kEnemy)) {
        isHit   = true;
        isAlive = false;
    }
}

void PlayerBullet::DrawImGuiGlobal() {
#ifdef USE_IMGUI
    ImGui::Begin("PlayerBullet Parameter (Global)");
    ImGui::SliderFloat("Speed",            &s_bulletSpeed,     0.1f,  10.0f,  "%.2f");
    ImGui::SliderFloat("Disappear Radius", &s_disappearRadius, 10.0f, 500.0f, "%.1f");
    ImGui::SliderFloat3("Scale",           &s_scale.x,         0.1f,  5.0f,   "%.2f");
    ImGui::SliderFloat3("Rotation",        &s_rotation.x,      -3.14f, 3.14f, "%.2f");
    ImGui::SliderInt("Damage",             &s_damage,          1, 100);
    ImGui::SliderFloat("Fire Interval",    &s_fireInterval,    0.01f, 1.0f,   "%.2f");
    ImGui::End();
#endif
}
