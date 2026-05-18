#include "PlayerCircusBullet.h"
#include "Character/Enemy/Enemy.h"
#include "Collider/CollisionTypeId.h"
#include <cmath>
#include <random>
#include "engine/graphic/Particle/ParticleManager.h"

namespace {
    constexpr float kFixedDeltaTime = 1.0f / 60.0f;
}

#include "Character/Enemy/CircusEnemy.h"

void PlayerCircusBullet::Initialize(const TuboEngine::Math::Vector3& startPos) {
    InitializeCircus(startPos, nullptr, nullptr);
}

void PlayerCircusBullet::InitializeCircus(const TuboEngine::Math::Vector3& startPos, TuboEngine::Camera* camera, CircusEnemy* boss) {
    // まず継承元の初期化を呼び、object3d 等を生成させる
    PlayerBullet::Initialize(startPos);

    // 以降、パラメータをサーカス弾用に上書きする
    Collider::SetTypeID(static_cast<uint32_t>(CollisionTypeId::kPlayerWeapon));
    SetPosition(startPos);
    Collider::SetRadius(0.8f);

    elapsedTime_ = 0.0f;
    if (boss) {
        speed_ = boss->GetBulletSpeed();
        turnSpeed_ = boss->GetBulletTurnSpeed();
        chaosAmplitude_ = boss->GetBulletChaosAmp();
        chaosFrequency_ = boss->GetBulletChaosFreq();
        phase1Duration_ = boss->GetBulletPhase1Duration();
    } else {
        speed_ = 1.0f;
        turnSpeed_ = 0.12f;
        chaosAmplitude_ = 0.8f;
        chaosFrequency_ = 10.0f;
        phase1Duration_ = 0.4f;
    }

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dist(0.0f, 6.283185f);
    swerveOffset_ = {dist(gen), dist(gen), dist(gen)};

    // エフェクト生成
    trailEmitter_ = TuboEngine::ParticleManager::GetInstance()->Find("PlayerCircusTrail");
    if (!trailEmitter_) {
        ParticlePreset p{};
        p.name = "PlayerCircusTrail";
        p.texture = "particle.png"; 
        p.maxInstances = 4000;
        p.autoEmit = false;
        p.lifeMin = 0.4f;
        p.lifeMax = 0.7f;
        p.scaleStart = {0.4f, 0.4f, 0.4f};
        p.scaleEnd = {0.0f, 0.0f, 0.0f};
        p.colorStart = {0.9f, 0.9f, 1.0f, 0.5f};
        p.colorEnd = {0.4f, 0.4f, 1.0f, 0.0f};
        trailEmitter_ = TuboEngine::ParticleManager::GetInstance()->CreateEmitterByType("Default", p);
    }

    explosionEmitter_ = TuboEngine::ParticleManager::GetInstance()->Find("PlayerCircusExplosion");
    if (!explosionEmitter_) {
        ParticlePreset p{};
        p.name = "PlayerCircusExplosion";
        p.texture = "circle.png"; 
        p.maxInstances = 1000;
        p.autoEmit = false;
        p.lifeMin = 0.2f;
        p.lifeMax = 0.4f;
        p.scaleStart = {0.8f, 0.8f, 0.8f};
        p.scaleEnd = {0.0f, 0.0f, 0.0f};
        p.colorStart = {0.0f, 0.4f, 1.0f, 1.0f}; // プレイヤー用は青系に
        p.colorEnd = {0.0f, 0.0f, 1.0f, 0.0f};
        p.velMin = {-1.8f, -1.8f, -1.8f};
        p.velMax = {1.8f, 1.8f, 1.8f};
        explosionEmitter_ = TuboEngine::ParticleManager::GetInstance()->CreateEmitterByType("Primitive", p);
    }

    burnerEmitter_ = TuboEngine::ParticleManager::GetInstance()->Find("PlayerCircusBurner");
    if (!burnerEmitter_) {
        ParticlePreset p{};
        p.name = "PlayerCircusBurner";
        p.texture = "circle.png"; 
        p.maxInstances = 2000;
        p.autoEmit = false;
        p.lifeMin = 0.05f;
        p.lifeMax = 0.12f;
        p.scaleStart = {0.4f, 0.4f, 0.4f};
        p.scaleEnd = {0.0f, 0.0f, 0.0f};
        p.colorStart = {0.2f, 0.8f, 1.0f, 1.0f}; 
        p.colorEnd = {0.0f, 0.2f, 1.0f, 0.0f};   
        p.velMin = {-0.1f, -0.1f, -0.1f};
        p.velMax = {0.1f, 0.1f, 0.1f};
        burnerEmitter_ = TuboEngine::ParticleManager::GetInstance()->CreateEmitterByType("Default", p);
    }

    sparkEmitter_ = TuboEngine::ParticleManager::GetInstance()->Find("PlayerCircusSpark");
    if (!sparkEmitter_) {
        ParticlePreset p{};
        p.name = "PlayerCircusSpark";
        p.texture = "circle.png"; 
        p.maxInstances = 4000;
        p.autoEmit = false;
        p.lifeMin = 0.3f;
        p.lifeMax = 0.6f;
        p.scaleStart = {0.2f, 0.2f, 0.2f};
        p.scaleEnd = {0.0f, 0.0f, 0.0f};
        p.colorStart = {0.5f, 0.9f, 1.0f, 1.0f}; 
        p.colorEnd = {0.0f, 0.2f, 1.0f, 0.0f};
        p.velMin = {-1.0f, -1.0f, -4.0f}; 
        p.velMax = {1.0f, 1.0f, -1.0f};
        sparkEmitter_ = TuboEngine::ParticleManager::GetInstance()->CreateEmitterByType("Default", p);
    }

    // Object3dの見た目を上書き
    object3d->Initialize("block/block.obj");
    object3d->SetPosition(GetPosition());
    object3d->SetScale({0.5f, 0.5f, 0.5f}); 
    object3d->SetModelColor({0.0f, 0.6f, 1.0f, 1.0f}); // プレイヤー用シアン色
    
    // カメラを設定して Update
    if (camera) {
        SetCamera(camera);
        object3d->Update();
    }
}

Enemy* PlayerCircusBullet::FindNearestEnemy() {
    Enemy* nearest = nullptr;
    float minDistSq = 999999.0f;
    TuboEngine::Math::Vector3 pos = GetPosition();
    for (Enemy* enemy : enemies_) {
        if (!enemy || !enemy->GetIsAlive()) continue;
        TuboEngine::Math::Vector3 ePos = enemy->GetCenterPosition();
        float distSq = (ePos.x - pos.x) * (ePos.x - pos.x) + (ePos.y - pos.y) * (ePos.y - pos.y) + (ePos.z - pos.z) * (ePos.z - pos.z);
        if (distSq < minDistSq) {
            minDistSq = distSq;
            nearest = enemy;
        }
    }
    return nearest;
}

void PlayerCircusBullet::Update() {
    if (!GetIsAlive()) return;

    elapsedTime_ += kFixedDeltaTime;
    TuboEngine::Math::Vector3 pos = GetPosition();
    TuboEngine::Math::Vector3 vel = GetVelocity();

    // ターゲット更新
    if (!currentTarget_ || !currentTarget_->GetIsAlive()) {
        currentTarget_ = FindNearestEnemy();
    }

    if (elapsedTime_ < phase1Duration_) {
        vel.x *= 0.94f;
        vel.y *= 0.94f;
        vel.z *= 0.94f;
    } else if (currentTarget_) {
        TuboEngine::Math::Vector3 targetPos = currentTarget_->GetCenterPosition();
        TuboEngine::Math::Vector3 toTarget = targetPos - pos;
        TuboEngine::Math::Vector3 targetDir = TuboEngine::Math::Vector3::Normalize(toTarget);
        TuboEngine::Math::Vector3 currentDir = TuboEngine::Math::Vector3::Normalize(vel);
        
        // プレイヤーのカウンター弾として爽快感・精度を高めるための調整
        float distanceToTarget = toTarget.Length();
        float playerTurnSpeed = turnSpeed_ * 2.2f;    // 基本旋回速度を2.2倍に向上
        float playerChaosAmp = chaosAmplitude_ * 0.4f; // うねり幅を4割に抑えて直線的かつ綺麗に曲げる
        
        // 【ゲームフィール向上（近接補正）】
        // 敵に近づいた場合（距離6.0f以内）、ターゲットを回り込んで外さないようにうねりブレをゼロにフェードアウトさせ、
        // 旋回性能をさらに高めて確実にヒットさせるようにアシストします
        if (distanceToTarget < 6.0f) {
            float t = distanceToTarget / 6.0f; // 0.0f（密着）〜 1.0f（境界）
            playerChaosAmp *= t;               // 近づくほどブレがゼロになる
            playerTurnSpeed *= (2.0f - t);     // 近づくほど旋回力が最大2倍に高まる
        }
        
        float homingStrength = playerTurnSpeed * std::min(2.5f, (elapsedTime_ - phase1Duration_) * 3.0f);
        
        TuboEngine::Math::Vector3 chaosVec;
        chaosVec.x = std::sin(elapsedTime_ * chaosFrequency_ + swerveOffset_.x);
        chaosVec.y = std::sin(elapsedTime_ * chaosFrequency_ * 1.2f + swerveOffset_.y); 
        chaosVec.z = std::cos(elapsedTime_ * chaosFrequency_ * 0.8f + swerveOffset_.z);

        TuboEngine::Math::Vector3 finalDir = currentDir + (targetDir * homingStrength) + (chaosVec * playerChaosAmp * 0.1f);
        finalDir.Normalize();
        vel = finalDir * speed_ * 60.0f; 
    }

    pos += vel * kFixedDeltaTime;
    SetPosition(pos);
    SetVelocity(vel);

    if (trailEmitter_) {
        trailEmitter_->GetPreset().center = pos;
        trailEmitter_->Emit(1);
    }
    if (burnerEmitter_ && vel.LengthSquared() > 0.001f) {
        TuboEngine::Math::Vector3 backDir = TuboEngine::Math::Vector3::Normalize(vel) * -0.5f; 
        burnerEmitter_->GetPreset().center = pos + backDir;
        burnerEmitter_->Emit(1);
    }
    if (sparkEmitter_ && vel.LengthSquared() > 10.0f) {
        sparkEmitter_->GetPreset().center = pos;
        sparkEmitter_->Emit(1);
    }

    // 地面衝突判定 (プレイヤーのカウンター弾はボスに当たるまで粘らせるため判定を甘くする)
    if (pos.z <= -2.0f) {
        if (explosionEmitter_) {
            explosionEmitter_->GetPreset().center = pos;
            explosionEmitter_->Emit(15);
        }
        SetIsAlive(false);
    }
    
    // 消滅時間
    if (elapsedTime_ > 6.0f) {
        SetIsAlive(false);
    }

    // BaseのUpdateを呼ぶと、壁判定やisAliveの判定が行われる
    // BaseBullet(PlayerBullet)のUpdateでは velocity を使って位置を動かしてしまうので、
    // ここで位置を手動で動かした後、PlayerBullet::Updateを呼ぶと二重に動いてしまう。
    // PlayerBullet::Update は呼ばず、オブジェクトの姿勢と生存だけ管理する。
    if (object3d) {
        object3d->SetPosition(pos);
        if (vel.LengthSquared() > 0.001f) {
            TuboEngine::Math::Vector3 dir = TuboEngine::Math::Vector3::Normalize(vel);
            float yaw = std::atan2(dir.y, dir.x);
            float pitch = std::atan2(-dir.z, std::sqrt(dir.x * dir.x + dir.y * dir.y));
            object3d->SetRotation({pitch, 0.0f, yaw + 1.5708f}); 
        }
        object3d->Update();
    }
}

void PlayerCircusBullet::Draw() {
    if (GetIsAlive() && object3d) {
        object3d->Draw();
    }
}

void PlayerCircusBullet::OnCollision(Collider* other) {
    if (!other) return;
    
    uint32_t typeID = other->GetTypeID();
    if (typeID == static_cast<uint32_t>(CollisionTypeId::kEnemy)) {
        if (explosionEmitter_) {
            explosionEmitter_->GetPreset().center = GetPosition();
            explosionEmitter_->Emit(15);
        }
        // 当たったらPlayerBulletのOnCollisionに流す（isHitやisAliveフラグを更新させる）
        PlayerBullet::OnCollision(other);
    }
}
