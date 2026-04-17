#include "CircusEnemy.h"
#include "Character/Player/Player.h"
#include "ImguiManager.h"
#include <cmath>
#include <random>

void CircusEnemy::Initialize() {
    Enemy::Initialize();
    HP = 800; // ボス的な扱いであればHP多めに
    fireInterval_ = 4.0f;
    missileCount_ = 32;
    currentAttack_ = AttackType::Burst;
    autoCycle_ = true;
    
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

    // 攻撃サイクルの更新
    UpdateAttackCycle(dt);

    // 板野サーカス：各種攻撃実行
    TryFireMissiles(canSeePlayer, dt);

    // 基底クラス(Enemy)の更新
    Enemy::Update();

    // 基底クラスの単発弾を無効化
    if (bullet) {
        bullet->SetIsAlive(false); 
    }
    bulletTimer_ = 0.0f;
}

void CircusEnemy::UpdateAttackCycle(float dt) {
    if (!autoCycle_) return;

    cycleTimer_ += dt;
    if (cycleTimer_ >= kCycleInterval) {
        cycleTimer_ = 0.0f;
        
        // 次の攻撃へ
        int next = static_cast<int>(currentAttack_) + 1;
        if (next > static_cast<int>(AttackType::Targeted)) {
            next = 0;
        }
        currentAttack_ = static_cast<AttackType>(next);
    }
}

void CircusEnemy::TryFireMissiles(bool canSeePlayer, float dt) {
    if (!player_) return;

    if (canSeePlayer) {
        fireTimer_ += dt;
        if (fireTimer_ >= fireInterval_) {
            fireTimer_ = 0.0f;
            ShowExclamation(2.0f);
            ExecuteAttack(currentAttack_);
        }
    } else {
        fireTimer_ = std::max(0.0f, fireTimer_ - dt);
    }
}

void CircusEnemy::ExecuteAttack(AttackType type) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dist(-1.0f, 1.0f);

    switch (type) {
    case AttackType::Burst:
        for (int i = 0; i < missileCount_; ++i) {
            TuboEngine::Math::Vector3 dir;
            dir.x = dist(gen) * 2.5f;
            dir.y = dist(gen) * 2.5f;
            dir.z = dist(gen) * 0.5f + 2.0f;
            dir.Normalize();
            FireSingleMissile(dir, 35.0f + dist(gen) * 15.0f);
        }
        break;

    case AttackType::Spiral:
        for (int i = 0; i < missileCount_; ++i) {
            float angle = (2.0f * 3.14159f / missileCount_) * i + spiralAngle_;
            TuboEngine::Math::Vector3 dir;
            dir.x = std::cos(angle);
            dir.y = std::sin(angle);
            dir.z = 0.5f;
            dir.Normalize();
            FireSingleMissile(dir, 30.0f);
        }
        spiralAngle_ += 0.5f; // 次回用に角度をずらす
        break;

    case AttackType::Cross:
        for (int i = 0; i < 4; ++i) {
            float baseAngle = (3.14159f / 2.0f) * i;
            for (int j = 0; j < missileCount_ / 4; ++j) {
                float angle = baseAngle + dist(gen) * 0.2f;
                TuboEngine::Math::Vector3 dir;
                dir.x = std::cos(angle);
                dir.y = std::sin(angle);
                dir.z = 0.2f;
                dir.Normalize();
                FireSingleMissile(dir, 20.0f + j * 5.0f);
            }
        }
        break;

    case AttackType::Targeted: {
        TuboEngine::Math::Vector3 toPlayer = player_->GetPosition() - position;
        toPlayer.Normalize();
        for (int i = 0; i < missileCount_; ++i) {
            TuboEngine::Math::Vector3 dir = toPlayer;
            dir.x += dist(gen) * 0.3f;
            dir.y += dist(gen) * 0.3f;
            dir.z += dist(gen) * 0.3f;
            dir.Normalize();
            FireSingleMissile(dir, 40.0f + dist(gen) * 10.0f);
        }
        break;
    }
    }
}

void CircusEnemy::FireSingleMissile(const TuboEngine::Math::Vector3& launchDir, float speed) {
    TuboEngine::Math::Vector3 startPos = position;
    startPos.z += 2.5f;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dist(-1.0f, 1.0f);

    auto bullet = std::make_unique<CircusBullet>();
    bullet->SetPlayer(player_);
    bullet->SetCamera(camera_);
    bullet->Initialize(startPos);
    
    // ImGuiからの調整値を反映
    bullet->SetSpeed(bulletSpeed_);
    bullet->SetTurnSpeed(bulletTurnSpeed_);
    bullet->SetChaosAmplitude(bulletChaosAmp_);
    bullet->SetChaosFrequency(bulletChaosFreq_);

    // Aroundモードならプレイヤーの周囲に目標地点をオフセット
    if (bulletMode_ == 1 && player_) {
        TuboEngine::Math::Vector3 offset;
        offset.x = dist(gen) * 6.0f; // 半径6m程度の範囲
        offset.y = dist(gen) * 6.0f;
        offset.z = dist(gen) * 2.0f; 
        bullet->SetTargetOffset(offset);
    }
    
    bullet->SetInitialVelocity(launchDir * speed);
    
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
    
    if (ImGui::CollapsingHeader("Attack Settings", ImGuiTreeNodeFlags_DefaultOpen)) {
        const char* attackNames[] = { "Burst", "Spiral", "Cross", "Targeted" };
        int currentType = static_cast<int>(currentAttack_);
        if (ImGui::Combo("Attack Type", &currentType, attackNames, IM_ARRAYSIZE(attackNames))) {
            currentAttack_ = static_cast<AttackType>(currentType);
        }
        ImGui::Checkbox("Auto Cycle Attacks", &autoCycle_);
        ImGui::ProgressBar(cycleTimer_ / kCycleInterval, ImVec2(0.0f, 0.0f), "Cycle Timer");
        
        ImGui::Separator();
        ImGui::DragFloat("Fire Interval", &fireInterval_, 0.1f, 0.5f, 15.0f);
        ImGui::DragInt("Missile Count", &missileCount_, 1, 1, 128);
        
        const char* modes[] = { "Homing", "Around Player" };
        ImGui::Combo("Bullet Mode", &bulletMode_, modes, IM_ARRAYSIZE(modes));
    }

    if (ImGui::CollapsingHeader("Bullet Tuning", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::DragFloat("Base Speed", &bulletSpeed_, 0.01f, 0.1f, 2.0f);
        ImGui::DragFloat("Turn Speed", &bulletTurnSpeed_, 0.001f, 0.01f, 0.5f);
        ImGui::DragFloat("Chaos Amplitude", &bulletChaosAmp_, 0.01f, 0.0f, 2.0f);
        ImGui::DragFloat("Chaos Frequency", &bulletChaosFreq_, 0.1f, 0.0f, 20.0f);
    }

    if (ImGui::Button("Force Fire Now")) {
        ExecuteAttack(currentAttack_);
    }

    ImGui::End();
#endif
}
