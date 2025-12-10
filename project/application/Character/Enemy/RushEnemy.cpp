#include "RushEnemy.h"
#include "MapChip/MapChipField.h"
#include "Character/Player/Player.h"
#include "ImGuiManager.h"
#include "LineManager.h"
#include "Collider/CollisionTypeId.h"
#include <cmath>
#include <algorithm>

#include "WinApp.h"
#include "engine/camera/Camera.h"

// 角度正規化 [-PI, PI]
float RushEnemy::NormalizeAngle(float angle) {
    while (angle > DirectX::XM_PI) angle -= 2.0f * DirectX::XM_PI;
    while (angle < -DirectX::XM_PI) angle += 2.0f * DirectX::XM_PI;
    return angle;
}

// 単純な矩形衝突付き移動 (X/Y 分離 + サブステップ)
static void MoveWithCollisionImpl(Vector3& position, const Vector3& desiredMove, MapChipField* field) {
    if (!field) { position = position + desiredMove; return; }
    const float tile = MapChipField::GetBlockSize();
    const float width  = tile * 0.8f;
    const float height = tile * 0.8f;
    float moveLen2D = std::sqrt(desiredMove.x*desiredMove.x + desiredMove.y*desiredMove.y);
    int subSteps = std::max(1, int(std::ceil(moveLen2D / (tile * 0.5f))));
    Vector3 step = desiredMove / float(subSteps);
    for (int i=0;i<subSteps;++i) {
        Vector3 nextX = position; nextX.x += step.x; if (!field->IsRectBlocked(nextX,width,height)) position = nextX;
        Vector3 nextY = position; nextY.y += step.y; if (!field->IsRectBlocked(nextY,width,height)) position = nextY;
    }
}
void RushEnemy::MoveWithCollision(Vector3& positionRef, const Vector3& desiredMove, MapChipField* field) { MoveWithCollisionImpl(positionRef, desiredMove, field); }

void RushEnemy::Initialize() {
    Enemy::Initialize();
    shootDistance_ = 0.0f; moveSpeed_ = baseMoveSpeed_;
    isPreparing_ = false; isRushing_ = false; isStopping_ = false; isScanning_ = false;
    rushCooldownTimer_ = 0.0f; requireExitBeforeNextRush_ = false; lookAroundTimer_ = 0.0f;
    isReacting_ = false; reactionTimer_ = 0.0f; reactionDir_ = {0,0,0};
    endedRushWithoutWall_ = false; // 新規追加フラグ初期化
    rushStretchTimer_ = 0.0f;       // 伸び演出初期化
    lastReactionSource_ = ReactionSource::None; // 直前の反応元初期化
    // スタン初期化
    isStunned_ = false; stunTimer_ = 0.0f;
}

// -------------------------------------------------
// Update helper: Perception and timers
// -------------------------------------------------
void RushEnemy::UpdatePerceptionAndTimers(float dt, bool& canSeePlayer, float& distanceToPlayer) {
    distanceToPlayer = 0.0f;
    if (player_) {
        Vector3 toPlayer = player_->GetPosition() - position;
        distanceToPlayer = std::sqrt(toPlayer.x*toPlayer.x + toPlayer.y*toPlayer.y + toPlayer.z*toPlayer.z);
    }
    canSeePlayer = CanSeePlayer();
    if (canSeePlayer) {
        lastSeenPlayerPos = player_->GetPosition();
        lastSeenTimer = kLastSeenDuration;
        ClearPath();
    } else if (lastSeenTimer > 0.0f) {
        lastSeenTimer -= dt;
    }
    if (wasHit && state_ == State::Idle) state_ = State::Alert;

    // クールダウン更新
    if (rushCooldownTimer_ > 0.0f) {
        rushCooldownTimer_ -= dt;
        if (rushCooldownTimer_ < 0.0f) rushCooldownTimer_ = 0.0f;
    }
    // スタン判定: ノックバック(リアクション)が完全に終わってからクールダウン残りの間だけ硬直
    if (rushCooldownTimer_ > 0.0f) {
        if (!isReacting_) {
            isStunned_ = true;
            stunTimer_ = rushCooldownTimer_;
        } else {
            // リアクション中はスタン扱いにしない
            isStunned_ = false;
        }
    } else {
        if (isStunned_) { isStunned_ = false; stunTimer_ = 0.0f; }
    }

    if (requireExitBeforeNextRush_ && player_) {
        float exitThresh = rushTriggerDistance_ + exitHysteresis_;
        if (distanceToPlayer > exitThresh) requireExitBeforeNextRush_ = false;
    }

    // scanning sway (無効化したいなら早期returnにしても良い)
    if (isScanning_) {
        lookAroundTimer_ -= dt;
        if (lookAroundTimer_ <= 0.0f) { isScanning_ = false; state_ = State::Idle; }
        float t = std::max(0.0f, lookAroundTimer_) / std::max(0.0001f, lookAroundDuration_);
        float sway = std::sin((1.0f - t) * DirectX::XM_PI * 1.5f) * 0.35f;
        rotation.z = NormalizeAngle(rotation.z + sway * 0.02f);
    }
}

// -------------------------------------------------
// Update helper: State update by vision
// -------------------------------------------------
void RushEnemy::UpdateStateByVision(bool canSeePlayer, float distanceToPlayer) {
    if (isScanning_ || isPreparing_ || isRushing_ || isStopping_ || isReacting_) return;
    if (!player_) return;
    if (canSeePlayer) {
        if (distanceToPlayer > moveStartDistance_) {
            state_ = State::Idle;
        } else if (distanceToPlayer > rushTriggerDistance_) {
            // 突進トリガー外: 追跡は許可
            state_ = State::Chase;
        } else {
            // 突進トリガー内: 突進のみ許可（通常の追跡はしない）
            if (rushCooldownTimer_ <= 0.0f && !requireExitBeforeNextRush_) {
                state_ = State::Attack;
                isPreparing_ = true;
                prepareTimer_ = prepareDuration_;
                Vector3 dir = player_->GetPosition() - position; dir.z = 0.0f; float len = std::sqrt(dir.x*dir.x + dir.y*dir.y);
                if (len > 0.001f) rushDir_ = {dir.x/len, dir.y/len, 0.0f};
                rotation.z = std::atan2(rushDir_.y, rushDir_.x);
                endedRushWithoutWall_ = false;
            } else {
                // クールダウン中や退出条件未満のときはその場で待機
                state_ = State::Idle;
            }
        }
    } else if (lastSeenTimer > 0.0f) state_ = State::Alert;
    else if (state_ == State::Alert) state_ = State::LookAround;
    else if (state_ != State::LookAround) state_ = State::Idle;
}

// -------------------------------------------------
// Update helper: Facing rotation
// -------------------------------------------------
void RushEnemy::UpdateFacingWhenNeeded(bool canSeePlayer) {
    if (isRushing_ || isPreparing_ || isStopping_ || isScanning_ || isReacting_) return;
    if (!player_) return;
    if (!(state_ == State::Chase || state_ == State::Alert)) return;
    Vector3 targetPos = (canSeePlayer) ? player_->GetPosition() : lastSeenPlayerPos;
    Vector3 toTarget = targetPos - position;
    float angleZ = std::atan2(toTarget.y, toTarget.x);
    float diff = NormalizeAngle(angleZ - rotation.z);
    float maxTurn = turnSpeed_;
    if (std::fabs(diff) < maxTurn) rotation.z = angleZ;
    else { rotation.z += (diff > 0 ? 1 : -1) * maxTurn; rotation.z = NormalizeAngle(rotation.z); }
}

// -------------------------------------------------
// Update helper: Attack state
// -------------------------------------------------
void RushEnemy::UpdateAttackState(float dt) {
    if (isPreparing_) { HandlePrepare(dt); return; }
    if (isRushing_)   { HandleRushing(dt); return; }
    if (isStopping_) {
        // 停止終了後は見回しせずIdleへ
        stopTimer_ -= dt;
        if (stopTimer_ <= 0.0f) { isStopping_ = false; state_ = State::Idle; }
    }
    if (isReacting_)  { HandleReacting(dt); }
}

void RushEnemy::HandlePrepare(float dt) {
    prepareTimer_ -= dt;
    if (prepareTimer_ <= 0.0f) {
        isPreparing_ = false;
        isRushing_ = true;
        rushTimer_ = rushDuration_;
        endedRushWithoutWall_ = false;
        // 突進開始瞬間の伸び演出
        rushStretchTimer_ = rushStretchDuration_;
    }
}

bool RushEnemy::CheckWallHit(const Vector3& desiredMove, Vector3& outHitNormal) const {
    if (!mapChipField) return false;
    const float tile = MapChipField::GetBlockSize();
    const float w = tile * 0.8f; const float h = tile * 0.8f;
    Vector3 nextPos = position + desiredMove;
    if (!mapChipField->IsRectBlocked(nextPos, w, h)) return false;
    // 近似法：軸分離で詰まり方向から法線推定
    Vector3 testX = position + Vector3{desiredMove.x,0,0};
    Vector3 testY = position + Vector3{0,desiredMove.y,0};
    bool blockX = mapChipField->IsRectBlocked(testX, w, h);
    bool blockY = mapChipField->IsRectBlocked(testY, w, h);
    if (blockX && !blockY) outHitNormal = {(desiredMove.x>0?-1.0f:1.0f),0,0};
    else if (!blockX && blockY) outHitNormal = {0,(desiredMove.y>0?-1.0f:1.0f),0};
    else outHitNormal = {-rushDir_.x, -rushDir_.y, 0};
    return true;
}

void RushEnemy::HandleRushing(float dt) {
    Vector3 desiredMove{rushDir_.x*rushSpeed_, rushDir_.y*rushSpeed_, 0};
    Vector3 hitNormal{0,0,0};
    bool hitWall = CheckWallHit(desiredMove, hitNormal);
    if (!hitWall) {
        MoveWithCollision(position, desiredMove, mapChipField);
        rushTimer_ -= dt;
        if (rushTimer_ <= 0.0f) { isRushing_ = false; isStopping_ = true; stopTimer_ = stopDuration_; rushCooldownTimer_ = rushCooldownDuration_; requireExitBeforeNextRush_ = true; endedRushWithoutWall_ = true; }
    } else {
        Vector3 v = rushDir_;
        float nlen = std::sqrt(hitNormal.x*hitNormal.x + hitNormal.y*hitNormal.y);
        Vector3 n = (nlen>0.0001f)? Vector3{hitNormal.x/nlen, hitNormal.y/nlen, 0}: Vector3{-v.x,-v.y,0};
        float dot = v.x*n.x + v.y*n.y;
        Vector3 reflect{ v.x - 2.0f*dot*n.x, v.y - 2.0f*dot*n.y, 0 };
        reactionDir_ = reflect; isRushing_ = false; isReacting_ = true; reactionTimer_ = reactionDuration_;
        rushCooldownTimer_ = rushCooldownDuration_; requireExitBeforeNextRush_ = true; endedRushWithoutWall_ = false;
        lastReactionSource_ = ReactionSource::Wall; // 壁反射リアクション
        Vector3 bounce = reactionDir_ * reactionBackoffSpeed_;
        MoveWithCollision(position, bounce, mapChipField);
    }
}

void RushEnemy::HandleReacting(float dt) {
    float t = std::clamp(reactionTimer_/std::max(0.0001f,reactionDuration_),0.0f,1.0f);
    float sp = reactionBackoffSpeed_ * t;
    Vector3 mv = reactionDir_ * sp;
    MoveWithCollision(position, mv, mapChipField);
    reactionTimer_ -= dt;
    if (reactionTimer_ <= 0.0f) {
        isReacting_ = false;
        // プレイヤーにぶつかった時だけは振り向かない（向き/突進方向を維持）
        if (lastReactionSource_ == ReactionSource::Wall) {
            rotation.z = std::atan2(reactionDir_.y, reactionDir_.x);
            rushDir_ = reactionDir_;
        }
        // 見回しに入らずIdleへ
        isScanning_ = false;
        state_ = State::Idle;
        endedRushWithoutWall_ = false;
        // リアクションが終わった時点で、クールダウンが残っていればスタン開始
        if (rushCooldownTimer_ > 0.0f) {
            isStunned_ = true;
            stunTimer_ = rushCooldownTimer_;
        }
        lastReactionSource_ = ReactionSource::None;
    }
}

// -------------------------------------------------
// Visuals and debug
// -------------------------------------------------
void RushEnemy::ApplyChargeAndVisuals(float dt) {
    Vector3 visualScale = scale;
    // チャージ中は縮み + 赤み
    if (isPreparing_) {
        float prepT = std::clamp(1.0f - (prepareTimer_ / std::max(0.0001f, prepareDuration_)), 0.0f, 1.0f);
        float pulse = 0.18f * std::sin(prepT * DirectX::XM_PI * 2.0f);
        float sx = 1.0f - 0.25f * prepT + pulse * 0.5f; // 徐々に縮む + 少し脈動
        float sy = 1.0f - 0.25f * prepT - pulse * 0.3f;
        visualScale.x *= std::max(0.6f, sx);
        visualScale.y *= std::max(0.6f, sy);
    }
    // 突進開始直後の一瞬伸び（進行方向=ローカルXを伸ばす）
    if (rushStretchTimer_ > 0.0f) {
        float t = std::clamp(rushStretchTimer_ / std::max(0.0001f, rushStretchDuration_), 0.0f, 1.0f);
        float stretch = 0.35f * t;
        visualScale.x *= (1.0f + stretch);
        visualScale.y *= (1.0f - stretch * 0.5f);
        rushStretchTimer_ -= dt;
        if (rushStretchTimer_ < 0.0f) rushStretchTimer_ = 0.0f;
    }
    // 突進継続中はわずかに伸ばしてスピード感
    if (isRushing_) {
        float runningStretch = 0.12f;
        visualScale.x *= (1.0f + runningStretch);
        visualScale.y *= (1.0f - runningStretch * 0.5f);
    }

    object3d->SetPosition(position);
    object3d->SetRotation(rotation);
    object3d->SetScale(visualScale);
    object3d->SetCamera(camera_);
    object3d->Update();
}

void RushEnemy::DrawDebugGizmos() {
    const int div = 24; Vector4 triggerCol{0.3f,0.9f,0.3f,0.35f};
    for (int i=0;i<div;++i){ float a0=(2.0f*DirectX::XM_PI)*(float(i)/div); float a1=(2.0f*DirectX::XM_PI)*(float(i+1)/div); Vector3 p0=position+Vector3{std::cos(a0)*rushTriggerDistance_, std::sin(a0)*rushTriggerDistance_,0}; Vector3 p1=position+Vector3{std::cos(a1)*rushTriggerDistance_, std::sin(a1)*rushTriggerDistance_,0}; LineManager::GetInstance()->DrawLine(p0,p1,triggerCol);}    
    Vector4 dirCol = isPreparing_? Vector4{1,0.6f,0.2f,1} : (isRushing_? Vector4{1,0.2f,0.2f,1} : (isStopping_? Vector4{0.8f,0.8f,0.8f,1} : (isScanning_? Vector4{0.2f,0.7f,1.0f,1} : (rushCooldownTimer_>0.0f? Vector4{0.4f,0.4f,0.4f,1}:Vector4{0.4f,0.4f,0.9f,1}))));
    Vector3 head = position + Vector3{rushDir_.x*2.2f,rushDir_.y*2.2f,0}; LineManager::GetInstance()->DrawLine(position, head, dirCol);
    if (isPreparing_ && showDashPreview_) {
        Vector3 predicted=position+Vector3{rushDir_.x*rushSpeed_*rushDuration_,rushDir_.y*rushSpeed_*rushDuration_,0}; float previewR=0.9f; Vector4 previewCol{1,0.15f,0.15f,0.85f};
        for (int i=0;i<div;++i){ float a0=(2.0f*DirectX::XM_PI)*(float(i)/div); float a1=(2.0f*DirectX::XM_PI)*(float(i+1)/div); Vector3 p0=predicted+Vector3{std::cos(a0)*previewR,std::sin(a0)*previewR,0}; Vector3 p1=predicted+Vector3{std::cos(a1)*previewR,std::sin(a1)*previewR,0}; LineManager::GetInstance()->DrawLine(p0,p1,previewCol);} float cross=previewR; LineManager::GetInstance()->DrawLine(predicted+Vector3{-cross,0,0},predicted+Vector3{cross,0,0},previewCol); LineManager::GetInstance()->DrawLine(predicted+Vector3{0,-cross,0},predicted+Vector3{0,cross,0},previewCol); LineManager::GetInstance()->DrawLine(position,predicted,Vector4{1,0.3f,0.3f,0.3f});
    }
}

// -------------------------------------------------
// Collision helpers
// -------------------------------------------------
bool RushEnemy::HandleReactingEarly(Collider* other, uint32_t typeID) {
    if (!isReacting_) return false;
    if (typeID == static_cast<uint32_t>(CollisionTypeId::kPlayerWeapon)) {
        Enemy::OnCollision(other);
    }
    return true;
}

bool RushEnemy::HandlePlayerCollision(Collider* other) {
    Player* hitPlayer = dynamic_cast<Player*>(other);
    if (!hitPlayer) return false;
    Vector3 pPos = hitPlayer->GetPosition(); Vector3 dir = position - pPos; dir.z = 0.0f; float len = std::sqrt(dir.x*dir.x + dir.y*dir.y);
    if (len > 0.0001f) {
        Vector3 away{dir.x/len, dir.y/len, 0.0f};
        reactionDir_ = away;
        isRushing_ = false;
        isReacting_ = true;
        reactionTimer_ = reactionDuration_;
        rushCooldownTimer_ = rushCooldownDuration_;
        requireExitBeforeNextRush_ = true;
        endedRushWithoutWall_ = false;
        lastReactionSource_ = ReactionSource::Player; // プレイヤー接触リアクション
        Vector3 knock = reactionDir_ * reactionBackoffSpeed_;
        MoveWithCollision(position, knock, mapChipField);
    }
    return true;
}

void RushEnemy::HandleWeaponAfterRush(Collider* other, uint32_t typeID) {
    if (typeID != static_cast<uint32_t>(CollisionTypeId::kPlayerWeapon) || !endedRushWithoutWall_) return;
    // 一度のみ
    endedRushWithoutWall_ = false;
    isStopping_ = false;
    isScanning_ = false;

    if (!player_) { state_ = State::Idle; return; }

    // プレイヤーとの距離で分岐
    Vector3 toP = player_->GetPosition() - position; toP.z = 0.0f;
    float len = std::sqrt(toP.x*toP.x + toP.y*toP.y);
    if (len > rushTriggerDistance_) {
        // 突進可能範囲外：Chase は許可（即時移動はしない）
        state_ = State::Chase;
    } else {
        // 突進可能範囲内：突進以外で接近しない
        if (rushCooldownTimer_ <= 0.0f && !requireExitBeforeNextRush_) {
            Vector3 dir{ toP.x/len, toP.y/len, 0.0f };
            rushDir_ = dir;
            rotation.z = std::atan2(dir.y, dir.x);
            state_ = State::Attack;
            isPreparing_ = true;
            prepareTimer_ = prepareDuration_;
        } else {
            state_ = State::Idle; // クールダウン中は足を止める
        }
    }
}

void RushEnemy::Update() {
    if (!isAllive) { if (!deathEffectPlayed_) { EmitDeathParticle(); deathEffectPlayed_ = true; } if (deathEmitter_) deathEmitter_->GetPreset().center = position; return; }
    const float dt = 1.0f / 60.0f;

    // --- Enemyのノックバック適用を先頭で共有 ---
    ApplyKnockback(dt);

    bool canSeePlayer = false; float distanceToPlayer = 0.0f;
    UpdatePerceptionAndTimers(dt, canSeePlayer, distanceToPlayer);

    // 可視/不可視の瞬間イベントをRushEnemyでも処理（アイコンタイマー設定）
    wasJustFound_ = (!sawPlayerPrev_ && canSeePlayer);
    wasJustLost_  = (sawPlayerPrev_ && !canSeePlayer);
    sawPlayerPrev_ = canSeePlayer;
    if (wasJustFound_) { exclamationTimer_ = iconDuration_; }
    if (wasJustLost_)  { questionTimer_    = iconDuration_; }

    // タイマー減衰
    if (questionTimer_ > 0.0f) { questionTimer_ -= dt; if (questionTimer_ < 0.0f) questionTimer_ = 0.0f; }
    if (exclamationTimer_ > 0.0f) { exclamationTimer_ -= dt; if (exclamationTimer_ < 0.0f) exclamationTimer_ = 0.0f; }
    if (surpriseTimer_ > 0.0f) { surpriseTimer_ -= dt; if (surpriseTimer_ < 0.0f) surpriseTimer_ = 0.0f; }

    // スタン中は完全停止（向き/状態維持）。見た目更新のみして早期return。
    if (isStunned_) {
        object3d->SetPosition(position);
        object3d->SetRotation(rotation);
        object3d->SetScale(scale);
        object3d->SetCamera(camera_);
        object3d->Update();
        if (hitEmitter_) hitEmitter_->GetPreset().center = position;
        if (deathEmitter_ && !deathEffectPlayed_) deathEmitter_->GetPreset().center = position;
        wasHit = isHit; isHit = false;
        return;
    }

    // クールダウン/スタンが終了した直後に、突進可能範囲内なら即座に突進準備へ移行
    if (!isStunned_ && !isPreparing_ && !isRushing_ && !isReacting_ && !isStopping_) {
        if (rushCooldownTimer_ <= 0.0f && !requireExitBeforeNextRush_ && player_ && canSeePlayer) {
            Vector3 toP = player_->GetPosition() - position; toP.z = 0.0f;
            float len = std::sqrt(toP.x*toP.x + toP.y*toP.y);
            if (len <= rushTriggerDistance_ && len > 0.001f) {
                Vector3 dir{ toP.x/len, toP.y/len, 0.0f };
                rushDir_ = dir;
                rotation.z = std::atan2(dir.y, dir.x);
                state_ = State::Attack;
                isPreparing_ = true;
                prepareTimer_ = prepareDuration_;
            }
        }
    }

    UpdateStateByVision(canSeePlayer, distanceToPlayer);

    if (state_ != State::Attack && !isScanning_) { isPreparing_ = false; if (isRushing_) isRushing_ = false; if (isStopping_) isStopping_ = false; }

    UpdateFacingWhenNeeded(canSeePlayer);

    switch (state_) {
    case State::Idle: break;
    case State::Alert: break;
    case State::LookAround: state_ = State::Idle; break;
    case State::Patrol: state_ = State::Idle; break;
    case State::Chase: {
        if (player_) { Vector3 dir = player_->GetPosition() - position; dir.z = 0.0f; float len = std::sqrt(dir.x*dir.x + dir.y*dir.y); if (len > 0.1f) { dir.x/=len; dir.y/=len; Vector3 move{dir.x*moveSpeed_, dir.y*moveSpeed_,0}; MoveWithCollision(position, move, mapChipField); } }
        break; }
    case State::Attack: { UpdateAttackState(dt); break; }
    }

    // モデル・パーティクル・デバッグ描画
    ApplyChargeAndVisuals(dt);
    if (hitEmitter_) hitEmitter_->GetPreset().center = position;
    if (deathEmitter_ && !deathEffectPlayed_) deathEmitter_->GetPreset().center = position;
    if (!wasHit && isHit) EmitHitParticle(); wasHit = isHit; isHit = false;
    DrawDebugGizmos();
}

void RushEnemy::Draw() { if (!GetIsAllive()) return; if (object3d) object3d->Draw(); DrawViewCone(); DrawLastSeenMark(); DrawStateIcon(); }

void RushEnemy::DrawSprite() {
    if (!GetIsAllive()) return;
    if (!camera_) return; // カメラ必須

    // 頭上ワールド座標
    Vector3 world = position;
    world.y += iconOffsetY_;

    // ワールド→スクリーン座標変換
    const Matrix4x4& vp = camera_->GetViewProjectionMatrix();
    // 行列は左手座標系の想定。Vector4に拡張
    float x = world.x, y = world.y, z = world.z;
    float sx = vp.m[0][0]*x + vp.m[1][0]*y + vp.m[2][0]*z + vp.m[3][0];
    float sy = vp.m[0][1]*x + vp.m[1][1]*y + vp.m[2][1]*z + vp.m[3][1];
    float sw = vp.m[0][3]*x + vp.m[1][3]*y + vp.m[2][3]*z + vp.m[3][3];
    if (sw == 0.0f) return; sx/=sw; sy/=sw; // NDC
    int w = WinApp::GetInstance()->GetClientWidth();
    int h = WinApp::GetInstance()->GetClientHeight();
    float baseX = (sx * 0.5f + 0.5f) * float(w);
    float baseY = (-sy * 0.5f + 0.5f) * float(h);
    baseY += iconScreenOffsetY_; // スクリーンYの追加オフセット（上方向は負）

    if (sw < 0.0f) return;
    if (baseX < -50 || baseX > w + 50 || baseY < -50 || baseY > h + 50) { return; }

    bool showQ = (questionTimer_ > 0.0f && questionIcon_ != nullptr);
    bool showE = (exclamationTimer_ > 0.0f && exclamationIcon_ != nullptr);

    // 両方表示時は重ならないようにオフセット（順序: ！ → ？）
    if (showQ && showE) {
        float gap = std::max(8.0f, iconSize_.x * 0.6f);
        float ex = baseX - gap * 0.5f; // 左に！
        float qx = baseX + gap * 0.5f; // 右に？

        exclamationIcon_->SetPosition({ex, baseY});
        exclamationIcon_->SetSize(iconSize_);
        exclamationIcon_->Update();
        exclamationIcon_->Draw();

        questionIcon_->SetPosition({qx, baseY});
        questionIcon_->SetSize(iconSize_);
        questionIcon_->Update();
        questionIcon_->Draw();
        return;
    }

    if (showQ) {
        questionIcon_->SetPosition({baseX, baseY});
        questionIcon_->SetSize(iconSize_);
        questionIcon_->Update();
        questionIcon_->Draw();
    }
    if (showE) {
        exclamationIcon_->SetPosition({baseX, baseY});
        exclamationIcon_->SetSize(iconSize_);
        exclamationIcon_->Update();
        exclamationIcon_->Draw();
    }
}

void RushEnemy::DrawImGui() {
#ifdef USE_IMGUI
    ImGui::Begin("RushEnemy");
    ImGui::Text("Pos:(%.2f,%.2f,%.2f)", position.x, position.y, position.z);
    ImGui::Text("HP:%d", HP);
    ImGui::Text("State:%d", (int)state_);
    ImGui::Text("Preparing:%s", isPreparing_?"true":"false");
    ImGui::Text("Rushing:%s", isRushing_?"true":"false");
    ImGui::Text("Stopping:%s", isStopping_?"true":"false");
    ImGui::Text("Scanning:%s", isScanning_?"true":"false");
    ImGui::Text("Reacting:%s", isReacting_?"true":"false");
    ImGui::Text("EndedRushWithoutWall:%s", endedRushWithoutWall_?"true":"false");

    ImGui::Checkbox("ShowDashPreview", &showDashPreview_);

    ImGui::DragFloat("BaseMoveSpeed", &baseMoveSpeed_, 0.005f, 0.0f, 2.0f);
    ImGui::DragFloat("PrepareMoveSpeed", &prepareMoveSpeed_, 0.002f, 0.0f, 0.2f);
    ImGui::DragFloat("RushSpeed", &rushSpeed_, 0.01f, 0.0f, 3.0f);

    ImGui::DragFloat("RushTriggerDistance", &rushTriggerDistance_, 0.05f, 0.0f, 20.0f);
    ImGui::DragFloat("PrepareDuration", &prepareDuration_, 0.01f, 0.05f, 5.0f);
    ImGui::DragFloat("RushDuration", &rushDuration_, 0.01f, 0.05f, 5.0f);

    ImGui::DragFloat("StopDuration", &stopDuration_, 0.01f, 0.0f, 5.0f);
    ImGui::DragFloat("LookAroundDuration", &lookAroundDuration_, 0.01f, 0.0f, 5.0f);
    ImGui::DragFloat("RushCooldown", &rushCooldownDuration_, 0.01f, 0.0f, 10.0f);

    ImGui::DragFloat("ExitHysteresis", &exitHysteresis_, 0.01f, 0.0f, 5.0f);
    ImGui::DragFloat("TurnSpeed", &turnSpeed_, 0.001f, 0.0f, 1.0f);

    ImGui::DragFloat("ReactionDuration", &reactionDuration_, 0.01f, 0.0f, 3.0f);
    ImGui::DragFloat("ReactionBackoffSpeed", &reactionBackoffSpeed_, 0.005f, 0.0f, 3.0f);

    moveSpeed_ = baseMoveSpeed_;
    ImGui::End();
#endif
}

// OnCollision 仕様:
//  1) isReacting_ 中はプレイヤー武器ダメージのみ反映 (位置/向き不変)
//  2) 壁に当たらず突進がタイマー終了した後(endedRushWithoutWall_==true)の最初の弾被弾のみ追跡開始演出
//  3) プレイヤー直接接触はノックバックリアクション (突進中断)
void RushEnemy::OnCollision(Collider* other) {
    if (!other) return;
    uint32_t typeID = other->GetTypeID();
    if (HandleReactingEarly(other, typeID)) return;
    Enemy::OnCollision(other);
    if (HandlePlayerCollision(other)) return;
    HandleWeaponAfterRush(other, typeID);
}
