#include "CircusBullet.h"
#include "Character/Player/Player.h"
#include "Collider/CollisionTypeId.h"
#include <cmath>
#include <algorithm>

namespace {
    constexpr float kFixedDeltaTime = 1.0f / 60.0f;
}

void CircusBullet::Initialize(const TuboEngine::Math::Vector3& startPos) {
    Collider::SetTypeID(static_cast<uint32_t>(CollisionTypeId::kEnemyWeapon));
    position = startPos;
    isAlive = true;
    elapsedTime_ = 0.0f;
    Collider::SetRadius(0.8f);

    // イタノサーカスらしさを出すための極端なパラメータ設定
    speed_ = 1.0f;           // 速度を少し抑えて追尾を甘く
    turnSpeed_ = 0.12f;      // 旋回性能を落として甘く
    chaosAmplitude_ = 0.8f;  // 揺らぎを適度に
    chaosFrequency_ = 10.0f; // 振動数
    phase1Duration_ = 0.0f;  // 即座に追尾開始

    object3d = std::make_unique<TuboEngine::Object3d>();
    object3d->Initialize("block/block.obj");
    object3d->SetPosition(position);
    object3d->SetScale({0.5f, 0.5f, 0.5f}); // 大幅にスケールアップして視認性を確保
    object3d->SetModelColor({1.0f, 0.0f, 0.6f, 1.0f}); // 鮮烈なマゼンタ
    if (camera_) {
        object3d->SetCamera(camera_);
    }
    // 初回更新（描画前に必須）
    object3d->Update();
}

void CircusBullet::SetCamera(TuboEngine::Camera* camera) {
    camera_ = camera;
    if (object3d) {
        object3d->SetCamera(camera);
    }
}

void CircusBullet::Update() {
    if (!isAlive) return;

    elapsedTime_ += kFixedDeltaTime;

    // 即座に追尾フェーズ（以前のフェーズ1と溜めを削除）
    if (player_) {
        TuboEngine::Math::Vector3 targetPos = player_->GetCenterPosition() + targetOffset_;
        TuboEngine::Math::Vector3 toTarget = targetPos - position;
        
        TuboEngine::Math::Vector3 targetDir = TuboEngine::Math::Vector3::Normalize(toTarget);
        TuboEngine::Math::Vector3 currentDir = TuboEngine::Math::Vector3::Normalize(velocity);
        
        // 旋回強度（経過時間で少しずつ強める）
        float homingStrength = turnSpeed_ * std::min(1.5f, elapsedTime_ * 2.0f);
        
        // 直進 + ターゲットへの補正
        TuboEngine::Math::Vector3 finalDir = currentDir + (targetDir * homingStrength);
        finalDir.Normalize();
        
        velocity = finalDir * speed_ * 60.0f; 
    }
    position += velocity * kFixedDeltaTime;

    // プレイヤー直撃判定
    if (player_) {
        TuboEngine::Math::Vector3 pc = player_->GetCenterPosition();
        if (TuboEngine::Math::Vector3::Distance(position, pc) < 1.5f) {
            player_->OnCollision(this);
            isAlive = false;
        }
    }

    // 地面衝突判定 (Z軸が高さ)
    if (position.z <= 0.0f) {
        isAlive = false;
    }

    // 画面外や時間経過で消滅
    if (elapsedTime_ > 6.0f) isAlive = false;

    if (object3d) {
        object3d->SetPosition(position);
        if (velocity.LengthSquared() > 0.001f) {
            TuboEngine::Math::Vector3 dir = TuboEngine::Math::Vector3::Normalize(velocity);
            // X-Y平面上での向き（Yaw）と上下角（Pitch）を計算
            float yaw = std::atan2(dir.y, dir.x);
            float pitch = std::atan2(-dir.z, std::sqrt(dir.x * dir.x + dir.y * dir.y));
            // モデルが進行方向を向くように回転を設定
            object3d->SetRotation({pitch, 0.0f, yaw + 1.5708f}); 
        }
        object3d->Update();
    }
}

void CircusBullet::Draw() {
    if (isAlive && object3d) {
        object3d->Draw();
    }
}

void CircusBullet::OnCollision(Collider* other) {
    if (other && other->GetTypeID() == static_cast<uint32_t>(CollisionTypeId::kPlayer)) {
        isAlive = false;
    }
}

TuboEngine::Math::Vector3 CircusBullet::GetCenterPosition() const {
    return position;
}
