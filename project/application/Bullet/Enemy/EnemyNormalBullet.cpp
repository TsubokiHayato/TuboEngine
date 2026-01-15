#include "EnemyNormalBullet.h"
#include "Character/Player/Player.h"
#include "Collider/CollisionTypeId.h"
#include "TextureManager.h"
#include "ImGuiManager.h"
#include <cmath>

constexpr float kPI = 3.14159265358979323846f;

namespace {
constexpr float kDefaultBulletSpeed   = 0.5f;                       // デフォルトの弾の速度
constexpr Vector3 kDefaultBulletScale = {1.0f, 1.0f, 1.0f};         // デフォルトの弾の大きさ
constexpr Vector3 kDefaultBulletRotation = {0.0f, 0.0f, 0.0f};      // デフォルトの弾の回転
constexpr float kBulletDisappearZ = 100.0f;
}

float EnemyNormalBullet::s_bulletSpeed      = kDefaultBulletSpeed;
float EnemyNormalBullet::s_disappearZ       = kBulletDisappearZ;
Vector3 EnemyNormalBullet::s_scale          = kDefaultBulletScale;
Vector3 EnemyNormalBullet::s_rotation       = kDefaultBulletRotation;
float EnemyNormalBullet::s_disappearRadius  = 50.0f;
int   EnemyNormalBullet::s_damage           = 1;
float EnemyNormalBullet::s_fireInterval     = 1.0f;

//--------------------------------------------------
// 初期化
//--------------------------------------------------
void EnemyNormalBullet::Initialize(const Vector3& startPos) {
    Collider::SetTypeID(static_cast<uint32_t>(CollisionTypeId::kEnemyWeapon));
    position     = startPos;
    velocity     = {};
    isAlive      = true;
    bulletSpeed  = s_bulletSpeed;
    disappearZ   = s_disappearZ;
    scale        = s_scale;
    rotation     = s_rotation;

    // 弾の当たり半径を設定（必要に応じて調整）
    Collider::SetRadius(0.5f);

    object3d = std::make_unique<Object3d>();
    object3d->Initialize("star.obj");
    object3d->SetPosition(position);
    object3d->SetRotation(rotation);
    object3d->SetScale(scale);
}

//--------------------------------------------------
// 更新処理（壁に当たったら消える）
//--------------------------------------------------
void EnemyNormalBullet::Update() {
    if (!isAlive) { return; }

    isHit       = false;
    bulletSpeed = s_bulletSpeed;
    scale       = s_scale;
    rotation    = s_rotation;

    // 進行方向算出（EnemyのZ回転が+Xを0ラジアン基準なら cos/sin / 現在式を維持）
    float angle = enemyRotation_.z - DirectX::XM_PIDIV2; // -90度オフセット基準の既存式
    velocity.x  = -std::sinf(angle) * bulletSpeed;
    velocity.y  =  std::cosf(angle) * bulletSpeed;
    velocity.z  = 0.0f;

    Vector3 desiredMove = velocity;

    // タイルサイズ取得（なければ1.0f）
    float tileSize = (mapChipField_) ? MapChipField::GetBlockSize() : 1.0f;
    float moveLen2D = std::sqrt(desiredMove.x * desiredMove.x + desiredMove.y * desiredMove.y);
    int subSteps = std::max(1, int(std::ceil(moveLen2D / (tileSize * 0.5f))));
    Vector3 stepMove = desiredMove / float(subSteps);

    for (int i = 0; i < subSteps && isAlive; ++i) {
        Vector3 nextPos = position + stepMove;

        // マップ衝突判定（境界外 / Block に当たったら消滅）
        if (mapChipField_ && mapChipField_->IsBlocked(nextPos)) {
            isAlive = false;
            break;
        }

        position = nextPos;

        // プレイヤー手動ヒットチェック（確実に消す）
        if (player_) {
            // 半径はプレイヤー/弾のCollider設定に合わせて調整
            const float bulletR  = 0.5f;    // Initialize で SetRadius と一致させる
            const float playerR  = 1.0f;    // プレイヤー半径（未設定なら1.0f目安）
            Vector3 pc = player_->GetCenterPosition();
            float dx = position.x - pc.x;
            float dy = position.y - pc.y;
            float rSum = bulletR + playerR;
            if (dx*dx + dy*dy <= rSum*rSum) {
                // プレイヤーに通知してダメージ処理を実行させる
                player_->OnCollision(this);
                isHit  = true;
                isAlive = false;
                break;
            }
        }
    }

    // プレイヤーから一定距離で消滅
    if (isAlive && player_) {
        Vector3 p = player_->GetPosition();
        float dx = position.x - p.x;
        float dy = position.y - p.y;
        float dist = std::sqrt(dx * dx + dy * dy);
        if (dist > s_disappearRadius) {
            isAlive = false;
        }
    }

    // Z判定
    if (isAlive && position.z > s_disappearZ) {
        isAlive = false;
    }

    if (!isAlive) { return; }

    object3d->SetPosition(position);
    object3d->SetRotation(rotation);
    object3d->SetScale(scale);
    object3d->Update();
}

//--------------------------------------------------
// 描画
//--------------------------------------------------
void EnemyNormalBullet::Draw() {
    if (isAlive) {
        object3d->Draw();
    }
}

//--------------------------------------------------
// ImGui（グローバル設定）
//--------------------------------------------------
void EnemyNormalBullet::DrawImGuiGlobal() {
#ifdef USE_IMGUI
    ImGui::Begin("Enemy Normal Bullet");
    ImGui::Text("Enemy Normal Bullet Settings");
    ImGui::DragFloat("Speed", &s_bulletSpeed, 0.01f, 0.0f, 10.0f);
    ImGui::DragFloat("Disappear Radius", &s_disappearRadius, 0.1f, 0.0f, 200.0f);
    ImGui::DragFloat3("Scale", &s_scale.x, 0.01f);
    ImGui::DragFloat3("Rotation", &s_rotation.x, 0.01f);
    ImGui::End();
#endif
}

//--------------------------------------------------
// 衝突時（プレイヤーに当たったら消える）
//--------------------------------------------------
void EnemyNormalBullet::OnCollision(Collider* other) {
    if (other->GetTypeID() == static_cast<uint32_t>(CollisionTypeId::kPlayer)) {
        isHit = true;
        isAlive = false; // プレイヤーに当たったら弾は消滅
    }
}

//--------------------------------------------------
// 当たり判定中心
//--------------------------------------------------
Vector3 EnemyNormalBullet::GetCenterPosition() const {
    return position;
}



