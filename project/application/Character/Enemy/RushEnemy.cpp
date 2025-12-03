#include "RushEnemy.h"
#include "MapChip/MapChipField.h"
#include "Character/Player/Player.h"
#include "ImGuiManager.h"
#include "LineManager.h"
#include "Collider/CollisionTypeId.h"
#include <cmath>

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
}

void RushEnemy::Update() {
    if (!isAllive) { if (!deathEffectPlayed_) { EmitDeathParticle(); deathEffectPlayed_ = true; } if (deathEmitter_) deathEmitter_->GetPreset().center = position; return; }
    const float dt = 1.0f / 60.0f;

    float distanceToPlayer = 0.0f; Vector3 toPlayer{0,0,0};
    if (player_) { toPlayer = player_->GetPosition() - position; distanceToPlayer = std::sqrt(toPlayer.x*toPlayer.x + toPlayer.y*toPlayer.y + toPlayer.z*toPlayer.z); }

    bool canSeePlayer = CanSeePlayer();
    if (canSeePlayer) { lastSeenPlayerPos = player_->GetPosition(); lastSeenTimer = kLastSeenDuration; ClearPath(); }
    else if (lastSeenTimer > 0.0f) { lastSeenTimer -= dt; }

    if (wasHit && state_ == State::Idle) state_ = State::Alert;

    if (rushCooldownTimer_ > 0.0f) { rushCooldownTimer_ -= dt; if (rushCooldownTimer_ < 0.0f) rushCooldownTimer_ = 0.0f; }
    if (requireExitBeforeNextRush_ && player_) { float exitThresh = rushTriggerDistance_ + exitHysteresis_; if (distanceToPlayer > exitThresh) requireExitBeforeNextRush_ = false; }

    if (isScanning_) {
        lookAroundTimer_ -= dt; if (lookAroundTimer_ <= 0.0f) { isScanning_ = false; state_ = State::Idle; }
        float t = lookAroundTimer_ / lookAroundDuration_; float sway = std::sin((1.0f - t) * DirectX::XM_PI * 1.5f) * 0.35f; rotation.z = NormalizeAngle(rotation.z + sway * 0.02f);
    }

    if (!isScanning_ && !isPreparing_ && !isRushing_ && !isStopping_ && !isReacting_) {
        if (player_) {
            if (canSeePlayer) {
                if (distanceToPlayer > moveStartDistance_) state_ = State::Idle;
                else if (distanceToPlayer > rushTriggerDistance_) state_ = State::Chase;
                else {
                    if (rushCooldownTimer_ <= 0.0f && !requireExitBeforeNextRush_) {
                        state_ = State::Attack; isPreparing_ = true; prepareTimer_ = prepareDuration_;
                        Vector3 dir = player_->GetPosition() - position; dir.z = 0.0f; float len = std::sqrt(dir.x*dir.x + dir.y*dir.y); if (len > 0.001f) rushDir_ = {dir.x/len, dir.y/len, 0.0f};
                        rotation.z = std::atan2(rushDir_.y, rushDir_.x);
                        endedRushWithoutWall_ = false; // 新しい突進開始でリセット
                    } else state_ = State::Chase;
                }
            } else if (lastSeenTimer > 0.0f) state_ = State::Alert;
            else if (state_ == State::Alert) state_ = State::LookAround;
            else if (state_ != State::LookAround) state_ = State::Idle;
        }
    }

    if (state_ != State::Attack && !isScanning_) { isPreparing_ = false; if (isRushing_) isRushing_ = false; if (isStopping_) isStopping_ = false; }

    if (!isRushing_ && !isPreparing_ && !isStopping_ && !isScanning_ && !isReacting_ && player_ && (state_ == State::Chase || state_ == State::Alert)) {
        Vector3 targetPos = (canSeePlayer) ? player_->GetPosition() : lastSeenPlayerPos; Vector3 toTarget = targetPos - position; float angleZ = std::atan2(toTarget.y,toTarget.x); float diff = NormalizeAngle(angleZ - rotation.z); float maxTurn = turnSpeed_; if (std::fabs(diff) < maxTurn) rotation.z = angleZ; else { rotation.z += (diff>0?1:-1)*maxTurn; rotation.z = NormalizeAngle(rotation.z); }
    }

    switch (state_) {
    case State::Idle: break; case State::Alert: break; case State::LookAround: state_ = State::Idle; break; case State::Patrol: state_ = State::Idle; break;
    case State::Chase: {
        if (player_) { Vector3 dir = player_->GetPosition() - position; dir.z = 0.0f; float len = std::sqrt(dir.x*dir.x + dir.y*dir.y); if (len > 0.1f) { dir.x/=len; dir.y/=len; Vector3 move{dir.x*moveSpeed_, dir.y*moveSpeed_,0}; MoveWithCollision(position, move, mapChipField); } }
        break; }
    case State::Attack: {
        if (isPreparing_) { prepareTimer_ -= dt; if (prepareTimer_ <= 0.0f) { isPreparing_ = false; isRushing_ = true; rushTimer_ = rushDuration_; endedRushWithoutWall_ = false; } }
        else if (isRushing_) {
            Vector3 desiredMove{rushDir_.x*rushSpeed_, rushDir_.y*rushSpeed_, 0}; bool hitWall=false; Vector3 hitNormal{0,0,0};
            if (mapChipField) {
                const float tile = MapChipField::GetBlockSize(); const float w = tile*0.8f; const float h = tile*0.8f; Vector3 nextPos = position + desiredMove; if (mapChipField->IsRectBlocked(nextPos,w,h)) {
                    hitWall = true; Vector3 testX = position + Vector3{desiredMove.x,0,0}; Vector3 testY = position + Vector3{0,desiredMove.y,0}; bool blockX = mapChipField->IsRectBlocked(testX,w,h); bool blockY = mapChipField->IsRectBlocked(testY,w,h); if (blockX && !blockY) hitNormal = {(desiredMove.x>0?-1.0f:1.0f),0,0}; else if (!blockX && blockY) hitNormal = {0,(desiredMove.y>0?-1.0f:1.0f),0}; else hitNormal = {-rushDir_.x,-rushDir_.y,0}; }
            }
            if (!hitWall) {
                MoveWithCollision(position, desiredMove, mapChipField); rushTimer_ -= dt; if (rushTimer_ <= 0.0f) { isRushing_ = false; isStopping_ = true; stopTimer_ = stopDuration_; rushCooldownTimer_ = rushCooldownDuration_; requireExitBeforeNextRush_ = true; endedRushWithoutWall_ = true; }
            } else {
                Vector3 v = rushDir_; float nlen = std::sqrt(hitNormal.x*hitNormal.x + hitNormal.y*hitNormal.y); Vector3 n = (nlen>0.0001f)? Vector3{hitNormal.x/nlen, hitNormal.y/nlen,0}:Vector3{-v.x,-v.y,0}; float dot = v.x*n.x + v.y*n.y; Vector3 reflect{ v.x - 2.0f*dot*n.x, v.y - 2.0f*dot*n.y,0 }; reactionDir_ = reflect; isRushing_ = false; isReacting_ = true; reactionTimer_ = reactionDuration_; rushCooldownTimer_ = rushCooldownDuration_; requireExitBeforeNextRush_ = true; endedRushWithoutWall_ = false; Vector3 bounce = reactionDir_ * reactionBackoffSpeed_; MoveWithCollision(position, bounce, mapChipField);
            }
        }
        else if (isStopping_) { stopTimer_ -= dt; if (stopTimer_ <= 0.0f) { isStopping_ = false; isScanning_ = true; lookAroundTimer_ = lookAroundDuration_; } }
        if (isReacting_) { float t = std::clamp(reactionTimer_/std::max(0.0001f,reactionDuration_),0.0f,1.0f); float sp = reactionBackoffSpeed_ * t; Vector3 mv = reactionDir_ * sp; MoveWithCollision(position,mv,mapChipField); reactionTimer_ -= dt; if (reactionTimer_ <= 0.0f) { isReacting_ = false; rotation.z = std::atan2(reactionDir_.y,reactionDir_.x); rushDir_ = reactionDir_; isScanning_ = true; lookAroundTimer_ = lookAroundDuration_; state_ = State::Idle; endedRushWithoutWall_ = false; } }
        break; }
    }

    // チャージ演出: 体の伸縮（スケール変更）と擬似色の変化
    Vector3 visualScale = scale;
    Vector4 tintColor = {1.0f,1.0f,1.0f,1.0f}; // 擬似カラー（必要ならObject3dに反映できるAPIがあれば使用）
    if (isPreparing_) {
        float prepT = std::clamp(1.0f - (prepareTimer_ / std::max(0.0001f, prepareDuration_)), 0.0f, 1.0f);
        float pulse = 0.18f * std::sin(prepT * DirectX::XM_PI * 2.0f);
        float sx = 1.0f + pulse;
        float sy = 1.0f - pulse * 0.5f;
        visualScale.x *= sx;
        visualScale.y *= sy;
        // 赤みを帯びる
        tintColor = {1.0f, 0.6f + 0.4f*prepT, 0.3f + 0.2f*std::sin(prepT*DirectX::XM_PI), 1.0f};
    }

    object3d->SetPosition(position);
    object3d->SetRotation(rotation);
    object3d->SetScale(visualScale);
    object3d->SetCamera(camera_);
    object3d->Update();

    if (hitEmitter_) hitEmitter_->GetPreset().center = position; if (deathEmitter_ && !deathEffectPlayed_) deathEmitter_->GetPreset().center = position; if (!wasHit && isHit) EmitHitParticle(); wasHit = isHit; isHit = false;

    const int div = 24; Vector4 triggerCol{0.3f,0.9f,0.3f,0.35f}; for (int i=0;i<div;++i){ float a0=(2.0f*DirectX::XM_PI)*(float(i)/div); float a1=(2.0f*DirectX::XM_PI)*(float(i+1)/div); Vector3 p0=position+Vector3{std::cos(a0)*rushTriggerDistance_, std::sin(a0)*rushTriggerDistance_,0}; Vector3 p1=position+Vector3{std::cos(a1)*rushTriggerDistance_, std::sin(a1)*rushTriggerDistance_,0}; LineManager::GetInstance()->DrawLine(p0,p1,triggerCol);}    
    Vector4 dirCol = isPreparing_? Vector4{1,0.6f,0.2f,1} : (isRushing_? Vector4{1,0.2f,0.2f,1} : (isStopping_? Vector4{0.8f,0.8f,0.8f,1} : (isScanning_? Vector4{0.2f,0.7f,1.0f,1} : (rushCooldownTimer_>0.0f? Vector4{0.4f,0.4f,0.4f,1}:Vector4{0.4f,0.4f,0.9f,1}))));
    Vector3 head = position + Vector3{rushDir_.x*2.2f,rushDir_.y*2.2f,0}; LineManager::GetInstance()->DrawLine(position, head, dirCol);

    if (isPreparing_) {
        float progress = 1.0f - (prepareTimer_ / std::max(0.0001f, prepareDuration_)); float baseR=0.6f; float pulse=0.25f*std::sin(progress*DirectX::XM_PI*2.0f); float radius = baseR + pulse; Vector4 pulseCol{1.0f,0.5f+0.5f*std::sin(progress*DirectX::XM_PI),0.1f,0.9f};
        for (int i=0;i<div;++i){ float a0=(2.0f*DirectX::XM_PI)*(float(i)/div); float a1=(2.0f*DirectX::XM_PI)*(float(i+1)/div); Vector3 p0=position+Vector3{std::cos(a0)*radius,std::sin(a0)*radius,0}; Vector3 p1=position+Vector3{std::cos(a1)*radius,std::sin(a1)*radius,0}; LineManager::GetInstance()->DrawLine(p0,p1,pulseCol);}    
        if (showDashPreview_) { Vector3 predicted=position+Vector3{rushDir_.x*rushSpeed_*rushDuration_,rushDir_.y*rushSpeed_*rushDuration_,0}; float previewR=0.9f; Vector4 previewCol{1,0.15f,0.15f,0.85f}; for (int i=0;i<div;++i){ float a0=(2.0f*DirectX::XM_PI)*(float(i)/div); float a1=(2.0f*DirectX::XM_PI)*(float(i+1)/div); Vector3 p0=predicted+Vector3{std::cos(a0)*previewR,std::sin(a0)*previewR,0}; Vector3 p1=predicted+Vector3{std::cos(a1)*previewR,std::sin(a1)*previewR,0}; LineManager::GetInstance()->DrawLine(p0,p1,previewCol);} float cross=previewR; LineManager::GetInstance()->DrawLine(predicted+Vector3{-cross,0,0},predicted+Vector3{cross,0,0},previewCol); LineManager::GetInstance()->DrawLine(predicted+Vector3{0,-cross,0},predicted+Vector3{0,cross,0},previewCol); LineManager::GetInstance()->DrawLine(position,predicted,Vector4{1,0.3f,0.3f,0.3f}); }
    }
}

void RushEnemy::Draw() { if (!GetIsAllive()) return; if (object3d) object3d->Draw(); DrawViewCone(); DrawLastSeenMark(); DrawStateIcon(); }

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

    // リアクション演出中: HPのみ
    if (isReacting_) {
        if (typeID == static_cast<uint32_t>(CollisionTypeId::kPlayerWeapon)) {
            Enemy::OnCollision(other);
        }
        return;
    }

    // 基底ダメージ処理 (弾など)
    Enemy::OnCollision(other);

    // プレイヤー直接接触ノックバック
    if (Player* hitPlayer = dynamic_cast<Player*>(other)) {
        Vector3 pPos = hitPlayer->GetPosition(); Vector3 dir = position - pPos; dir.z = 0.0f; float len = std::sqrt(dir.x*dir.x + dir.y*dir.y);
        if (len > 0.0001f) {
            Vector3 away{dir.x/len, dir.y/len, 0.0f}; reactionDir_ = away; isRushing_ = false; isReacting_ = true; reactionTimer_ = reactionDuration_; rushCooldownTimer_ = rushCooldownDuration_; requireExitBeforeNextRush_ = true; endedRushWithoutWall_ = false; Vector3 knock = reactionDir_ * reactionBackoffSpeed_; MoveWithCollision(position, knock, mapChipField);
        }
        return;
    }

    // 壁非ヒット突進終了後の最初の弾被弾のみ追跡開始
    if (typeID == static_cast<uint32_t>(CollisionTypeId::kPlayerWeapon) && endedRushWithoutWall_) {
        endedRushWithoutWall_ = false; // 一度のみ
        isStopping_ = false; isScanning_ = false; state_ = State::Chase;
        if (player_) {
            Vector3 toP = player_->GetPosition() - position; toP.z = 0.0f; float len = std::sqrt(toP.x*toP.x + toP.y*toP.y); if (len > 0.0001f) { Vector3 dir{toP.x/len, toP.y/len, 0.0f}; rotation.z = std::atan2(dir.y, dir.x); Vector3 step{dir.x*baseMoveSpeed_, dir.y*baseMoveSpeed_, 0}; MoveWithCollision(position, step, mapChipField); }
        }
    }
}
