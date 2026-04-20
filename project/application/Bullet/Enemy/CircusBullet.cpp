#include "CircusBullet.h"
#include "Character/Player/Player.h"
#include "Collider/CollisionTypeId.h"
#include <cmath>
#include <algorithm>
#include <random>
#include "engine/graphic/Particle/ParticleManager.h"

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
    phase1Duration_ = 0.4f;  // 発射後0.4秒間のタメを作る

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dist(0.0f, 6.283185f); // 0 ~ 2PI
    swerveOffset_ = {dist(gen), dist(gen), dist(gen)};

    // トレイルエミッターの取得・生成（全ミサイルで共有し、大量生成による負荷とリークを防ぐ）
    trailEmitter_ = TuboEngine::ParticleManager::GetInstance()->Find("CircusSharedTrail");
    if (!trailEmitter_) {
        ParticlePreset p{};
        p.name = "CircusSharedTrail";
        p.texture = "particle.png"; 
        p.maxInstances = 4000;
        p.autoEmit = false;
        p.lifeMin = 0.4f;
        p.lifeMax = 0.7f;
        p.scaleStart = {0.4f, 0.4f, 0.4f};
        p.scaleEnd = {0.0f, 0.0f, 0.0f};
        p.colorStart = {0.9f, 0.9f, 0.9f, 0.5f};
        p.colorEnd = {0.4f, 0.4f, 0.4f, 0.0f};
        trailEmitter_ = TuboEngine::ParticleManager::GetInstance()->CreateEmitterByType("Default", p);
    }

    explosionEmitter_ = TuboEngine::ParticleManager::GetInstance()->Find("CircusExplosion");
    if (!explosionEmitter_) {
        ParticlePreset p{};
        p.name = "CircusExplosion";
        p.texture = "circle.png"; 
        p.maxInstances = 1000;
        p.autoEmit = false;
        p.lifeMin = 0.2f;
        p.lifeMax = 0.4f;
        p.scaleStart = {0.8f, 0.8f, 0.8f};
        p.scaleEnd = {0.0f, 0.0f, 0.0f};
        p.colorStart = {1.0f, 0.4f, 0.0f, 1.0f}; // 激しいオレンジ色
        p.colorEnd = {1.0f, 0.0f, 0.0f, 0.0f};
        p.velMin = {-1.8f, -1.8f, -1.8f};
        p.velMax = {1.8f, 1.8f, 1.8f};
        explosionEmitter_ = TuboEngine::ParticleManager::GetInstance()->CreateEmitterByType("Primitive", p);
    }

    burnerEmitter_ = TuboEngine::ParticleManager::GetInstance()->Find("CircusSharedBurner");
    if (!burnerEmitter_) {
        ParticlePreset p{};
        p.name = "CircusSharedBurner";
        p.texture = "circle.png"; 
        p.maxInstances = 2000;
        p.autoEmit = false;
        p.lifeMin = 0.05f;
        p.lifeMax = 0.12f; // 短命な炎
        p.scaleStart = {0.4f, 0.4f, 0.4f};
        p.scaleEnd = {0.0f, 0.0f, 0.0f};
        p.colorStart = {1.0f, 0.8f, 0.2f, 1.0f}; // 強烈な黄色～オレンジ
        p.colorEnd = {1.0f, 0.2f, 0.0f, 0.0f};   // 赤くなって消える
        p.velMin = {-0.1f, -0.1f, -0.1f};
        p.velMax = {0.1f, 0.1f, 0.1f};
        burnerEmitter_ = TuboEngine::ParticleManager::GetInstance()->CreateEmitterByType("Default", p);
    }

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

    if (elapsedTime_ < phase1Duration_) {
        // 発射直後は空気抵抗のように減速しつつ飛散する（タメフェーズ）
        velocity.x *= 0.94f;
        velocity.y *= 0.94f;
        velocity.z *= 0.94f;
    } else if (player_) {
        // タメが終わったら一気に加速＆ホーミングを開始
        TuboEngine::Math::Vector3 targetPos = player_->GetCenterPosition() + targetOffset_;
        TuboEngine::Math::Vector3 toTarget = targetPos - position;
        
        TuboEngine::Math::Vector3 targetDir = TuboEngine::Math::Vector3::Normalize(toTarget);
        TuboEngine::Math::Vector3 currentDir = TuboEngine::Math::Vector3::Normalize(velocity);
        
        // 旋回強度（タメ終了後からの経過時間で徐々に強める）
        float homingStrength = turnSpeed_ * std::min(1.5f, (elapsedTime_ - phase1Duration_) * 2.0f);
        
        // カオス軌道の計算（ウネウネ）
        // 各軸に異なる位相・周波数のサイン波をかけて大きく揺らぐベクトルを作る
        TuboEngine::Math::Vector3 chaosVec;
        chaosVec.x = std::sin(elapsedTime_ * chaosFrequency_ + swerveOffset_.x);
        chaosVec.y = std::sin(elapsedTime_ * chaosFrequency_ * 1.2f + swerveOffset_.y); // 軸によって周波数や位相を少しずらす
        chaosVec.z = std::cos(elapsedTime_ * chaosFrequency_ * 0.8f + swerveOffset_.z);

        // 直進 + ターゲットへの補正 + ウネウネ補正
        TuboEngine::Math::Vector3 finalDir = currentDir + (targetDir * homingStrength) + (chaosVec * chaosAmplitude_ * 0.1f);
        finalDir.Normalize();
        
        // ホーミング開始後はスピードリミットへ
        velocity = finalDir * speed_ * 60.0f; 
    }
    position += velocity * kFixedDeltaTime;

    // トレイル（煙）とバーナー（推進器の炎）の発生
    if (isAlive) {
        if (trailEmitter_) {
            trailEmitter_->GetPreset().center = position;
            trailEmitter_->Emit(1);
        }
        if (burnerEmitter_ && velocity.LengthSquared() > 0.001f) {
            // ミサイルの進行方向の逆側に少しずらして炎を吹かせる
            TuboEngine::Math::Vector3 backDir = TuboEngine::Math::Vector3::Normalize(velocity) * -0.5f; 
            burnerEmitter_->GetPreset().center = position + backDir;
            burnerEmitter_->Emit(1);
        }
    }

    // プレイヤー直撃判定
    bool hit = false;
    if (player_) {
        TuboEngine::Math::Vector3 pc = player_->GetCenterPosition();
        if (TuboEngine::Math::Vector3::Distance(position, pc) < 1.5f) {
            player_->OnCollision(this);
            isAlive = false;
            hit = true;
        }
    }

    // 地面衝突判定 (Z軸が高さ)
    if (isAlive && position.z <= 0.0f) {
        isAlive = false;
        hit = true;
    }

    // 画面外や時間経過で消滅
    if (isAlive && elapsedTime_ > 6.0f) isAlive = false;

    // 着弾時の爆発エフェクト
    if (hit && explosionEmitter_) {
        explosionEmitter_->GetPreset().center = position;
        explosionEmitter_->Emit(15);
    }

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
        if (isAlive && explosionEmitter_) {
            explosionEmitter_->GetPreset().center = position;
            explosionEmitter_->Emit(15);
        }
        isAlive = false;
    }
}

TuboEngine::Math::Vector3 CircusBullet::GetCenterPosition() const {
    return position;
}
