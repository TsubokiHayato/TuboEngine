#include "CircusEnemy.h"
#include "Character/Player/Player.h"
#include "ImguiManager.h"
#include <cmath>
#include <random>

void CircusEnemy::Initialize() {
    Enemy::Initialize();
    HP = 400; 
    fireInterval_ = 6.0f;
    missileCount_ = 32; 
    
    if (object3d) {
        object3d->SetModelColor({1.0f, 0.0f, 0.7f, 1.0f});
    }
}

void CircusEnemy::Update() {
    const float dt = 1.0f / 60.0f;

    if (isDying_) {
        for (auto& b : bullets_) {
            if (b) b->Update();
        }
        Enemy::Update();
        return;
    }

    if (!isAlive || HP <= 0) {
        Enemy::Update();
        return;
    }

    bool canSeePlayer = CanSeePlayer();
    
    // 弾の更新
    for (auto it = bullets_.begin(); it != bullets_.end(); ) {
        if ((*it)->GetIsAlive()) {
            (*it)->Update();
            ++it;
        } else {
            it = bullets_.erase(it);
        }
    }

    // 板野サーカス：一斉射撃（爆発的拡散）
    TryFireMissiles(canSeePlayer, dt);

    // 基底クラス(Enemy)の更新
    Enemy::Update();

    // 基底クラスの単発弾を無効化
    if (bullet) {
        bullet->SetIsAlive(false); 
    }
    bulletTimer_ = 0.0f;
}

void CircusEnemy::TryFireMissiles(bool canSeePlayer, float dt) {
    if (!player_) return;

    if (canSeePlayer) {
        fireTimer_ += dt;
        if (fireTimer_ >= fireInterval_) {
            fireTimer_ = 0.0f;
            ShowExclamation(2.0f);
            
            // 一気に全弾発射して爆発的な広がりを作る
            for (int i = 0; i < missileCount_; ++i) {
                FireSingleMissile();
            }
        }
    } else {
        fireTimer_ = std::max(0.0f, fireTimer_ - dt);
    }
}

void CircusEnemy::FireSingleMissile() {
    TuboEngine::Math::Vector3 startPos = position;
    startPos.z += 2.5f;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dist(-1.0f, 1.0f);

    auto bullet = std::make_unique<CircusBullet>();
    bullet->SetPlayer(player_);
    bullet->SetCamera(camera_);
    bullet->Initialize(startPos);
    
    // Aroundモードならプレイヤーの周囲に目標地点をオフセット
    if (bulletMode_ == 1 && player_) {
        TuboEngine::Math::Vector3 offset;
        offset.x = dist(gen) * 6.0f; // 半径6m程度の範囲
        offset.y = dist(gen) * 6.0f;
        offset.z = dist(gen) * 2.0f; 
        bullet->SetTargetOffset(offset);
    }
    
    // 拡散方向をより劇的に
    TuboEngine::Math::Vector3 initDir;
    initDir.x = dist(gen) * 2.5f; 
    initDir.y = dist(gen) * 2.5f; 
    initDir.z = dist(gen) * 0.5f + 2.0f; 
    
    initDir.Normalize();
    float launchSpeed = 35.0f + dist(gen) * 15.0f; // さらに爆発的な初速
    bullet->SetInitialVelocity(initDir * launchSpeed);
    
    bullets_.push_back(std::move(bullet));
}

void CircusEnemy::Draw() {
    Enemy::Draw();
    for (auto& b : bullets_) {
        if (b) b->Draw();
    }
}

void CircusEnemy::DrawImGui() {
#ifdef USE_IMGUI
    ImGui::Begin("CircusEnemy");
    ImGui::Text("Remaining Missiles: %d", remainingMissiles_);
    
    // 挙動モードの切り替え
    const char* modes[] = { "Homing", "Around Player" };
    ImGui::Combo("Bullet Mode", &bulletMode_, modes, IM_ARRAYSIZE(modes));
    
    ImGui::DragFloat("Fire Interval", &fireInterval_, 0.1f, 1.0f, 15.0f);
    ImGui::DragInt("Missile Count", &missileCount_, 1, 1, 128);
    ImGui::End();
#endif
}
