#include "CircusEnemy.h"
#include "Character/Player/Player.h"
#include "ImguiManager.h"
#include <cmath>
#include <random>
#include "engine/graphic/Particle/ParticleManager.h"

void CircusEnemy::Initialize() {
    Enemy::Initialize();
    maxHp_ = 100; // HPを800から350程度に低下（調整可能）
    HP = maxHp_;
    animatedHp_ = static_cast<float>(maxHp_);

    fireInterval_ = 4.0f;
    missileCount_ = 32;
    currentAttack_ = AttackType::Burst;
    autoCycle_ = true;
    
    if (object3d) {
        object3d->SetModelColor({1.0f, 0.0f, 0.7f, 1.0f});
    }

    chargeEmitter_ = TuboEngine::ParticleManager::GetInstance()->Find("CircusCharge");
    if (!chargeEmitter_) {
        ParticlePreset p{};
        p.name = "CircusCharge";
        p.texture = "gradationLine.png"; 
        p.maxInstances = 20;
        p.autoEmit = false;
        p.lifeMin = 0.5f;
        p.lifeMax = 0.8f;
        p.scaleStart = {4.0f, 4.0f, 1.0f}; // 大きなリング
        p.scaleEnd = {0.0f, 0.0f, 1.0f};   // 小さく収縮する
        p.colorStart = {1.0f, 0.2f, 0.8f, 0.0f};
        p.colorEnd = {1.0f, 0.5f, 1.0f, 0.8f};
        chargeEmitter_ = TuboEngine::ParticleManager::GetInstance()->CreateEmitterByType("Ring", p);
    }

    muzzleFlashEmitter_ = TuboEngine::ParticleManager::GetInstance()->Find("CircusMuzzleFlash");
    if (!muzzleFlashEmitter_) {
        ParticlePreset p{};
        p.name = "CircusMuzzleFlash";
        p.texture = "circle.png"; 
        p.maxInstances = 400;
        p.autoEmit = false;
        p.lifeMin = 0.2f;
        p.lifeMax = 0.5f;
        p.scaleStart = {2.0f, 2.0f, 2.0f};
        p.scaleEnd = {0.0f, 0.0f, 0.0f};
        p.colorStart = {1.0f, 1.0f, 1.0f, 1.0f}; // 白い煙
        p.colorEnd = {0.8f, 0.8f, 0.8f, 0.0f};
        p.velMin = {-3.0f, -3.0f, -0.5f};
        p.velMax = {3.0f, 3.0f, 4.0f};
        muzzleFlashEmitter_ = TuboEngine::ParticleManager::GetInstance()->CreateEmitterByType("Primitive", p);
    }

    auraEmitter_ = TuboEngine::ParticleManager::GetInstance()->Find("CircusEnemyAura");
    if (!auraEmitter_) {
        ParticlePreset p{};
        p.name = "CircusEnemyAura";
        p.texture = "circle.png";
        p.maxInstances = 500;
        p.autoEmit = false;
        p.lifeMin = 0.4f;
        p.lifeMax = 0.8f;
        p.scaleStart = {0.8f, 0.8f, 0.8f};
        p.scaleEnd = {0.1f, 0.1f, 0.1f};
        p.colorStart = {1.0f, 0.0f, 0.0f, 0.8f};
        p.colorEnd = {0.5f, 0.0f, 1.0f, 0.0f};
        p.velMin = {-1.5f, -1.5f, 1.0f};
        p.velMax = {1.5f, 1.5f, 4.0f};
        auraEmitter_ = TuboEngine::ParticleManager::GetInstance()->CreateEmitterByType("Default", p);
    }

    explosionEmitter_ = TuboEngine::ParticleManager::GetInstance()->Find("CircusEnemyExplosion");
    if (!explosionEmitter_) {
        ParticlePreset p{};
        p.name = "CircusEnemyExplosion";
        p.texture = "circle.png";
        p.maxInstances = 400;
        p.autoEmit = false;
        p.lifeMin = 0.5f;
        p.lifeMax = 1.0f;
        p.scaleStart = {3.0f, 3.0f, 3.0f};
        p.scaleEnd = {0.0f, 0.0f, 0.0f};
        p.colorStart = {1.0f, 0.5f, 0.0f, 1.0f};
        p.colorEnd = {1.0f, 0.0f, 0.0f, 0.0f};
        p.velMin = {-3.0f, -3.0f, -3.0f};
        p.velMax = {3.0f, 3.0f, 3.0f};
        explosionEmitter_ = TuboEngine::ParticleManager::GetInstance()->CreateEmitterByType("Primitive", p);
    }

    // ボス用巨大HPバーのスプライト初期化
    bossHpFrameSprite_ = std::make_unique<TuboEngine::Sprite>();
    bossHpFrameSprite_->Initialize("HpBarFrame.png");
    bossHpFrameSprite_->SetSize(hpFrameSizeBase_);
    bossHpFrameSprite_->SetPosition({640.0f, 60.0f});   // 画面上部中央 (1280x720前提)
    bossHpFrameSprite_->SetAnchorPoint({0.5f, 0.5f});

    // 背景の遅延用バー（被弾時に現れる）
    bossHpBarBgSprite_ = std::make_unique<TuboEngine::Sprite>();
    bossHpBarBgSprite_->Initialize("Hp.png");
    bossHpBarBgSprite_->SetAnchorPoint({0.0f, 0.5f});
    bossHpBarBgSprite_->SetPosition({640.0f - 300.0f, 60.0f});
    bossHpBarBgSprite_->SetSize({600.0f, 32.0f});
    bossHpBarBgSprite_->SetColor({1.0f, 0.8f, 0.0f, 1.0f}); // 黄色など

    bossHpBarSprite_ = std::make_unique<TuboEngine::Sprite>();
    bossHpBarSprite_->Initialize("Hp.png");
    bossHpBarSprite_->SetAnchorPoint({0.0f, 0.5f});     // 左から伸縮するように左端アンカー
    bossHpBarSprite_->SetPosition({640.0f - 300.0f, 60.0f}); // 中央から半幅引いた位置
    bossHpBarSprite_->SetSize({600.0f, 32.0f});
    bossHpBarSprite_->SetColor({1.0f, 0.0f, 0.2f, 1.0f}); // ボスらしく赤紫系に

    // UIパーティクルプールの初期化
    hpParticles_.clear();
    for (int i = 0; i < 40; ++i) {
        SimpleUIParticle p;
        p.sprite = std::make_unique<TuboEngine::Sprite>();
        p.sprite->Initialize("circle.png");
        p.sprite->SetSize({8.0f, 8.0f});
        p.sprite->SetAnchorPoint({0.5f, 0.5f});
        p.active = false;
        hpParticles_.push_back(std::move(p));
    }
}

void CircusEnemy::Update() {
    const float dt = 1.0f / 60.0f;

    // ボスHPバーの更新
    // 基準位置
    float baseX = 640.0f;
    float baseY = 60.0f;

    // Frameの位置とサイズ
    if (bossHpFrameSprite_) {
        bossHpFrameSprite_->SetPosition({baseX + hpFrameOffset_.x, baseY + hpFrameOffset_.y});
        bossHpFrameSprite_->SetSize(hpFrameSizeBase_);
        bossHpFrameSprite_->Update();
    }

    // Barの基準（左端座標）
    float barLeftX = baseX - hpBarSizeBase_.x * 0.5f + hpBarOffset_.x;
    float barY = baseY + hpBarOffset_.y;

    float hpRatio = static_cast<float>(HP) / static_cast<float>(maxHp_); 
    hpRatio = std::clamp(hpRatio, 0.0f, 1.0f);

    // HP減少時のアニメーション＆パーティクル処理
    if (animatedHp_ > static_cast<float>(HP)) {
        animatedHp_ -= static_cast<float>(maxHp_) * 0.5f * dt; // 1秒で最大値の50%分減る
        if (animatedHp_ < static_cast<float>(HP)) animatedHp_ = static_cast<float>(HP);

        // パーティクルの発生
        hpParticleTimer_ += dt;
        if (hpParticleTimer_ > 0.03f) { // 細かく放出
            hpParticleTimer_ = 0.0f;
            float animRatio = animatedHp_ / static_cast<float>(maxHp_);
            TuboEngine::Math::Vector2 emitPos = {barLeftX + hpBarSizeBase_.x * animRatio, barY};
            EmitHpParticle(emitPos); // 右端から出す
            EmitHpParticle(emitPos); // 追加で出す
        }
    } else {
        animatedHp_ = static_cast<float>(HP); // 回復した場合などはぱっと合わせる
    }

    float animatedHpRatio = animatedHp_ / static_cast<float>(maxHp_);
    animatedHpRatio = std::clamp(animatedHpRatio, 0.0f, 1.0f);

    if (bossHpBarBgSprite_) {
        // 背景（遅延バー）は animatedHp_ を反映
        bossHpBarBgSprite_->SetPosition({barLeftX, barY});
        bossHpBarBgSprite_->SetSize({hpBarSizeBase_.x * animatedHpRatio, hpBarSizeBase_.y});
        bossHpBarBgSprite_->Update();
    }

    if (bossHpBarSprite_) {
        // 赤バー（本体）は現在の HP を反映
        bossHpBarSprite_->SetPosition({barLeftX, barY});
        bossHpBarSprite_->SetSize({hpBarSizeBase_.x * hpRatio, hpBarSizeBase_.y});
        bossHpBarSprite_->Update();
    }

    // UIパーティクル更新
    UpdateHpParticles(dt);

    // デスフラグのトリガー
    if (HP <= 0 && !isDying_) {
        isDying_ = true;
        deathTimer_ = 3.0f; // 3秒かけて爆発
        nextExplosionTime_ = 0.0f;
    }

    if (isDying_) {
        for (auto& b : bullets_) {
            if (b && b->GetIsAlive()) b->Update();
        }

        deathTimer_ -= dt;
        nextExplosionTime_ -= dt;

        // 連鎖小爆発
        if (nextExplosionTime_ <= 0.0f) {
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_real_distribution<float> dist(-2.0f, 2.0f);

            if (explosionEmitter_) {
                TuboEngine::Math::Vector3 expPos = position;
                expPos.x += dist(gen);
                expPos.y += dist(gen);
                expPos.z += dist(gen) + 2.0f; // 腰から頭あたり
                explosionEmitter_->GetPreset().center = expPos;
                explosionEmitter_->Emit(10);
            }
            std::uniform_real_distribution<float> timeDist(0.05f, 0.25f);
            nextExplosionTime_ = timeDist(gen);
        }

        // 最後に大爆発して消える
        if (deathTimer_ <= 0.0f) {
            if (explosionEmitter_) {
                explosionEmitter_->GetPreset().center = position;
                explosionEmitter_->GetPreset().scaleStart = {12.0f, 12.0f, 12.0f};
                explosionEmitter_->Emit(80);
                explosionEmitter_->GetPreset().scaleStart = {3.0f, 3.0f, 3.0f}; // 元に戻す
            }
            isAlive = false;
        }

        Enemy::Update();
        return;
    }

    if (!isAlive) {
        Enemy::Update();
        return;
    }

    // 激怒オーラ (HPが半分以下になったら常時発生)
    if (HP <= maxHp_ / 2) {
        if (auraEmitter_) {
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_real_distribution<float> dist(-1.5f, 1.5f);

            TuboEngine::Math::Vector3 auraPos = position;
            auraPos.x += dist(gen);
            auraPos.y += dist(gen);
            auraPos.z += std::abs(dist(gen)) * 1.5f + 1.0f;

            auraEmitter_->GetPreset().center = auraPos;
            auraEmitter_->Emit(2); // 毎フレーム少しずつ
        }
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

void CircusEnemy::EmitHpParticle(const TuboEngine::Math::Vector2& pos) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dist(-1.0f, 1.0f);
    std::uniform_real_distribution<float> angleDist(0.0f, 3.14159f * 2.0f);

    for (auto& p : hpParticles_) {
        if (!p.active) {
            p.active = true;
            // 右端から出るのでランダムに少し上下に散らす
            p.position = {pos.x, pos.y + dist(gen) * 16.0f}; 
            float angle = angleDist(gen);
            // 速度（飛び散る早さ）
            float speed = 50.0f + std::abs(dist(gen)) * 100.0f;
            // UI上の座標系は下が+Yなので、上方向への重力成分や拡散
            p.velocity = {std::cos(angle) * speed, std::sin(angle) * speed - 50.0f}; 
            p.life = 0.3f + std::abs(dist(gen)) * 0.3f;
            p.maxLife = p.life;
            // キラキラした黄色〜赤色
            p.color = {1.0f, 0.4f + std::abs(dist(gen)) * 0.6f, 0.0f, 1.0f};
            p.sprite->SetColor(p.color);
            break;
        }
    }
}

void CircusEnemy::UpdateHpParticles(float dt) {
    for (auto& p : hpParticles_) {
        if (p.active) {
            p.life -= dt;
            if (p.life <= 0.0f) {
                p.active = false;
            } else {
                p.position.x += p.velocity.x * dt;
                p.position.y += p.velocity.y * dt;
                // アルファフェード
                float alpha = p.life / p.maxLife;
                TuboEngine::Math::Vector4 c = p.color;
                c.w = alpha;
                // 重力（下方向＝Y+に少し落ちる）
                p.velocity.y += 150.0f * dt; 
                p.sprite->SetColor(c);
                p.sprite->SetPosition(p.position);
                p.sprite->Update();
            }
        }
    }
}

void CircusEnemy::DrawHpParticles() {
    for (auto& p : hpParticles_) {
        if (p.active) {
            p.sprite->Draw();
        }
    }
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
        
        // 発射の1秒前にチャージエフェクトを出す
        if (fireTimer_ >= fireInterval_ - 1.0f && !isCharging_) {
            isCharging_ = true;
            if (chargeEmitter_) {
                chargeEmitter_->GetPreset().center = position + TuboEngine::Math::Vector3{0.0f, 0.0f, 2.5f};
                chargeEmitter_->Emit(3); // リングを数個重ねて吸い込み感を出す
            }
        }

        if (fireTimer_ >= fireInterval_) {
            fireTimer_ = 0.0f;
            isCharging_ = false; // チャージ状態リセット
            ShowExclamation(2.0f);
            
            // マズルフラッシュ（激しい排煙）エフェクト
            if (muzzleFlashEmitter_) {
                muzzleFlashEmitter_->GetPreset().center = position + TuboEngine::Math::Vector3{0.0f, 0.0f, 2.5f};
                muzzleFlashEmitter_->Emit(60);
            }
            
            ExecuteAttack(currentAttack_);
        }
    } else {
        fireTimer_ = std::max(0.0f, fireTimer_ - dt);
        if (fireTimer_ < fireInterval_ - 1.0f) {
            isCharging_ = false; // 見失ってチャージがキャンセルされたらリセット
        }
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
    bullet->SetPhase1Duration(bulletPhase1Duration_);
    bullet->SetTargetDelayFrames(bulletTargetDelayFrames_); // ★追加

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

void CircusEnemy::DrawSprite() {
    if (!isAlive) return;

    if (bossHpFrameSprite_) bossHpFrameSprite_->Draw();
    if (bossHpBarBgSprite_) bossHpBarBgSprite_->Draw(); // 遅れて減るバーを追加
    if (bossHpBarSprite_) bossHpBarSprite_->Draw();
    DrawHpParticles();
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
        ImGui::DragFloat("Delay (Phase1)", &bulletPhase1Duration_, 0.05f, 0.0f, 2.0f);
        ImGui::DragInt("Target Delay Frames", &bulletTargetDelayFrames_, 1, 0, 180, "%d frames");
    }

    if (ImGui::CollapsingHeader("HP UI Tuning", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::DragFloat2("Frame Offset", &hpFrameOffset_.x, 1.0f);
        ImGui::DragFloat2("Frame Size", &hpFrameSizeBase_.x, 1.0f);
        ImGui::DragFloat2("Bar Offset", &hpBarOffset_.x, 1.0f);
        ImGui::DragFloat2("Bar Size", &hpBarSizeBase_.x, 1.0f);

        if (ImGui::Button("Test Damage (-50 HP)")) {
            HP -= 50;
            if (HP < 0) HP = 0;
            bulletTimer_ = 0.0f; // 更新のため
        }
    }

    if (ImGui::Button("Force Fire Now")) {
        ExecuteAttack(currentAttack_);
    }

    ImGui::End();
#endif
}
