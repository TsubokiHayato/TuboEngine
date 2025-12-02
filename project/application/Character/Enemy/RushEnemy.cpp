#include "RushEnemy.h"
#include "MapChip/MapChipField.h"
#include "Character/Player/Player.h"
#include "ImGuiManager.h"
#include "LineManager.h"
#include <cmath>

// 角度正規化
float RushEnemy::NormalizeAngle(float angle) { while (angle > DirectX::XM_PI) angle -= 2.0f * DirectX::XM_PI; while (angle < -DirectX::XM_PI) angle += 2.0f * DirectX::XM_PI; return angle; }

// Enemy.cpp のロジック簡易移植
static void MoveWithCollisionImpl(Vector3& position, const Vector3& desiredMove, MapChipField* field) {
    if (!field) { position = position + desiredMove; return; }
    const float tile = MapChipField::GetBlockSize();
    const float width = tile * 0.8f;
    const float height = tile * 0.8f;
    float moveLen2D = std::sqrt(desiredMove.x * desiredMove.x + desiredMove.y * desiredMove.y);
    int subSteps = std::max(1, int(std::ceil(moveLen2D / (tile * 0.5f))));
    Vector3 step = desiredMove / float(subSteps);
    for (int i = 0; i < subSteps; ++i) {
        Vector3 nextX = position; nextX.x += step.x; if (!field->IsRectBlocked(nextX, width, height)) { position = nextX; }
        Vector3 nextY = position; nextY.y += step.y; if (!field->IsRectBlocked(nextY, width, height)) { position = nextY; }
    }
}
void RushEnemy::MoveWithCollision(Vector3& positionRef, const Vector3& desiredMove, MapChipField* field) { MoveWithCollisionImpl(positionRef, desiredMove, field); }

void RushEnemy::Initialize() {
    Enemy::Initialize();
    shootDistance_ = 0.0f; // 射撃しない
    moveSpeed_ = baseMoveSpeed_;
    isPreparing_ = false;
    isRushing_ = false;
    isStopping_ = false;
    rushCooldownTimer_ = 0.0f;
    requireExitBeforeNextRush_ = false;
}

void RushEnemy::Update() {
    if (!isAllive) {
        if (!deathEffectPlayed_) { EmitDeathParticle(); deathEffectPlayed_ = true; }
        if (deathEmitter_) { deathEmitter_->GetPreset().center = position; }
        return;
    }
    const float dt = 1.0f / 60.0f;

    float distanceToPlayer = 0.0f; Vector3 toPlayer = {0,0,0}; if (player_) { toPlayer = player_->GetPosition() - position; distanceToPlayer = std::sqrt(toPlayer.x*toPlayer.x + toPlayer.y*toPlayer.y + toPlayer.z*toPlayer.z); }
    bool canSeePlayer = CanSeePlayer();
    if (canSeePlayer) { lastSeenPlayerPos = player_->GetPosition(); lastSeenTimer = kLastSeenDuration; ClearPath(); } else if (lastSeenTimer > 0.0f) { lastSeenTimer -= dt; }

    // 攻撃されたら気づく
    if (wasHit && state_ == State::Idle) { state_ = State::Alert; }

    // クールダウン
    if (rushCooldownTimer_ > 0.0f) {
        rushCooldownTimer_ -= dt;
        if (rushCooldownTimer_ < 0.0f) rushCooldownTimer_ = 0.0f;
    }

    // ヒステリシス: トリガー外へ一度出るまで次突進禁止
    if (requireExitBeforeNextRush_ && player_) {
        float exitThresh = rushTriggerDistance_ + exitHysteresis_;
        if (distanceToPlayer > exitThresh) {
            requireExitBeforeNextRush_ = false; // 外に出たので次突進許可
        }
    }

    // ユーザー仕様に沿ったシーケンス
    // 1) 視界に入る -> Enemy::CanSeePlayer() を使用
    // 2) プレイヤー方向を向く (準備中も) / 3) チャージ (向き固定) / 4) 突進 / 5) 少し止まる / 6) クールダウン後に見回す
    if (player_) {
        if (canSeePlayer) {
            if (!isPreparing_ && !isRushing_ && !isStopping_) {
                if (distanceToPlayer > moveStartDistance_) {
                    state_ = State::Idle;
                } else if (distanceToPlayer > rushTriggerDistance_) {
                    // 近づくのみ（視界維持）
                    state_ = State::Chase;
                } else {
                    // チャージ開始（角度決定後は固定）
                    if (rushCooldownTimer_ <= 0.0f && !requireExitBeforeNextRush_) {
                        state_ = State::Attack; // Attackでまとめる
                        isPreparing_ = true;
                        prepareTimer_ = prepareDuration_;
                        // 2) 突進するプレイヤーの方向を向く
                        Vector3 dir = player_->GetPosition() - position; dir.z = 0.0f; float len = std::sqrt(dir.x*dir.x + dir.y*dir.y);
                        if (len > 0.001f) { rushDir_ = {dir.x/len, dir.y/len, 0.0f}; }
                        float angleZ = std::atan2(rushDir_.y, rushDir_.x);
                        rotation.z = angleZ; // 向きを即合わせる
                    } else {
                        state_ = State::Chase;
                    }
                }
            }
        } else if (lastSeenTimer > 0.0f) {
            state_ = State::Alert;
        } else if (state_ == State::Alert) {
            state_ = State::LookAround;
        } else if (state_ != State::LookAround) {
            state_ = State::Idle;
        }
    }

    // Attack以外になったら準備/突進/停止フェーズ解除
    if (state_ != State::Attack) {
        isPreparing_ = false;
        if (isRushing_) isRushing_ = false;
        if (isStopping_) isStopping_ = false;
    }

    // 回転: チャージ開始時に角度確定、準備中・停止中は角度維持、Chase/Alertは追尾で回す、突進中は固定
    if (!isRushing_ && !isPreparing_ && !isStopping_ && player_ && (state_ == State::Chase || state_ == State::Alert)) {
        Vector3 targetPos = (canSeePlayer) ? player_->GetPosition() : lastSeenPlayerPos; Vector3 toTarget = targetPos - position; float angleZ = std::atan2(toTarget.y, toTarget.x); float diff = NormalizeAngle(angleZ - rotation.z); float maxTurn = turnSpeed_; if (std::fabs(diff) < maxTurn) rotation.z = angleZ; else { rotation.z += (diff > 0 ? 1 : -1) * maxTurn; rotation.z = NormalizeAngle(rotation.z); }
    }

    switch (state_) {
    case State::Idle: break;
    case State::Alert: break;
    case State::LookAround: {
        // 6) 見回す: 既存LookAroundロジックを簡略、一定時間経過でIdleへ
        // ここではEnemy既存のLookAroundパラメータを活用したいが簡略のためIdleへ戻す
        state_ = State::Idle;
        break; }
    case State::Patrol: state_ = State::Idle; break;
    case State::Chase: {
        if (player_) {
            Vector3 dir = player_->GetPosition() - position; dir.z = 0.0f; float len = std::sqrt(dir.x*dir.x + dir.y*dir.y); if (len > 0.1f) { dir.x/=len; dir.y/=len; Vector3 desiredMove{dir.x*moveSpeed_, dir.y*moveSpeed_, 0.0f}; MoveWithCollision(position, desiredMove, mapChipField); }
        }
        break; }
    case State::Attack: {
        if (isPreparing_) {
            // 3) チャージ中: 角度固定、わずかに前進
            Vector3 desiredMove{rushDir_.x*prepareMoveSpeed_, rushDir_.y*prepareMoveSpeed_, 0.0f};
            MoveWithCollision(position, desiredMove, mapChipField);
            prepareTimer_ -= dt;
            if (prepareTimer_ <= 0.0f) {
                // 4) 突進開始
                isPreparing_ = false;
                isRushing_ = true;
                rushTimer_ = rushDuration_;
            }
        } else if (isRushing_) {
            // 4) 突進: 固定方向へ移動
            Vector3 desiredMove{rushDir_.x*rushSpeed_, rushDir_.y*rushSpeed_, 0.0f};
            MoveWithCollision(position, desiredMove, mapChipField);
            rushTimer_ -= dt;
            if (rushTimer_ <= 0.0f) {
                // 5) 停止フェーズ突入
                isRushing_ = false;
                isStopping_ = true;
                stopTimer_ = stopDuration_;
                rushCooldownTimer_ = rushCooldownDuration_; // クールダウン開始
                requireExitBeforeNextRush_ = true;          // ヒステリシス
            }
        } else if (isStopping_) {
            // 5) 停止: 位置固定（見た目はその場で停止）
            stopTimer_ -= dt;
            if (stopTimer_ <= 0.0f) {
                isStopping_ = false;
                // 6) クールダウンが終わるまでLookAroundし、終わればIdleへ
                if (rushCooldownTimer_ > 0.0f) {
                    state_ = State::LookAround;
                } else {
                    state_ = State::Idle;
                }
            }
        }
        break; }
    }

    object3d->SetPosition(position); object3d->SetRotation(rotation); object3d->SetScale(scale); object3d->SetCamera(camera_); object3d->Update();
    if (hitEmitter_) hitEmitter_->GetPreset().center = position;
    if (deathEmitter_ && !deathEffectPlayed_) deathEmitter_->GetPreset().center = position;
    if (!wasHit && isHit) { EmitHitParticle(); }
    wasHit = isHit; isHit = false;

    // Debug Lines: トリガー円 + 突進方向
    const int div = 24; Vector4 guideCol = {0.3f, 0.9f, 0.3f, 0.6f};
    for (int i = 0; i < div; ++i) {
        float a0 = (2.0f * DirectX::XM_PI) * (float(i) / div);
        float a1 = (2.0f * DirectX::XM_PI) * (float(i + 1) / div);
        Vector3 p0 = position + Vector3{ std::cos(a0) * rushTriggerDistance_, std::sin(a0) * rushTriggerDistance_, 0.0f };
        Vector3 p1 = position + Vector3{ std::cos(a1) * rushTriggerDistance_, std::sin(a1) * rushTriggerDistance_, 0.0f };
        LineManager::GetInstance()->DrawLine(p0, p1, guideCol);
    }
    Vector4 dirCol = isPreparing_ ? Vector4{1.0f, 0.6f, 0.0f, 1.0f} : (isRushing_ ? Vector4{1.0f, 0.2f, 0.2f, 1.0f} : (isStopping_ ? Vector4{0.6f, 0.6f, 0.6f, 1.0f} : (rushCooldownTimer_>0.0f? Vector4{0.4f,0.4f,0.4f,1.0f}:Vector4{0.4f,0.4f,0.9f,1.0f})));
    Vector3 head = position + Vector3{rushDir_.x * 2.0f, rushDir_.y * 2.0f, 0.0f};
    LineManager::GetInstance()->DrawLine(position, head, dirCol);
}

void RushEnemy::Draw() {
    if (!GetIsAllive()) return; if (object3d) object3d->Draw(); DrawViewCone(); DrawLastSeenMark(); DrawStateIcon();
}

void RushEnemy::DrawImGui() {
#ifdef USE_IMGUI
    ImGui::Begin("RushEnemy");
    ImGui::Text("Pos:(%.2f,%.2f,%.2f)", position.x, position.y, position.z);
    ImGui::Text("HP:%d", HP);
    ImGui::Text("Alive:%s", GetIsAllive()?"Yes":"No");
    ImGui::Text("State:%d", (int)state_);
    ImGui::Text("Preparing:%s", isPreparing_?"true":"false");
    ImGui::Text("Rushing:%s", isRushing_?"true":"false");
    ImGui::Text("Stopping:%s", isStopping_?"true":"false");
    ImGui::DragFloat("BaseMoveSpeed", &baseMoveSpeed_, 0.01f, 0.0f, 2.0f);
    ImGui::DragFloat("PrepareMoveSpeed", &prepareMoveSpeed_, 0.005f, 0.0f, 0.3f);
    ImGui::DragFloat("RushSpeed", &rushSpeed_, 0.01f, 0.0f, 3.0f);
    ImGui::DragFloat("RushTriggerDistance", &rushTriggerDistance_, 0.05f, 0.0f, 20.0f);
    ImGui::DragFloat("PrepareDuration", &prepareDuration_, 0.01f, 0.05f, 3.0f);
    ImGui::DragFloat("RushDuration", &rushDuration_, 0.01f, 0.05f, 3.0f);
    ImGui::DragFloat("StopDuration", &stopDuration_, 0.01f, 0.0f, 3.0f);
    ImGui::DragFloat("RushCooldown", &rushCooldownDuration_, 0.01f, 0.0f, 5.0f);
    ImGui::DragFloat("ExitHysteresis", &exitHysteresis_, 0.01f, 0.0f, 5.0f);
    ImGui::DragFloat("TurnSpeed", &turnSpeed_, 0.001f, 0.0f, 1.0f);
    moveSpeed_ = baseMoveSpeed_;
    ImGui::End();
#endif
}
