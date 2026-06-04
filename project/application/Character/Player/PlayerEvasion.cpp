#include "PlayerEvasion.h"
#include <cmath>
#include "GameConstants.h"

using GameConstants::kFixedDeltaTime;

PlayerEvasion::PlayerEvasion() {}

void PlayerEvasion::Update() {
    if (dodgeCooldownTimer_ > 0.0f) {
        dodgeCooldownTimer_ -= kFixedDeltaTime;
        if (dodgeCooldownTimer_ < 0.0f) {
            dodgeCooldownTimer_ = 0.0f;
        }
    }

    // 回避開始からの経過時間をカウントアップ（回避中のみ）
    if (isDodging_) {
        justEvasionTimer_ += kFixedDeltaTime;
    }

    if (isDodging_) {
        dodgeTimer_ -= kFixedDeltaTime;
        if (dodgeTimer_ <= 0.0f) {
         isDodging_ = false;
            justEvasionTimer_ = 0.0f;
            hasJustEvaded_ = false;
        }
    }
}

void PlayerEvasion::StartDodge(const TuboEngine::Math::Vector3& inputDir) {
    if (dodgeCooldownTimer_ > 0.0f || isDodging_) {
        return; // クールダウン中、または既に回避中は不可（呼び出し元で制御してもよい）
    }

    isDodging_ = true;
    dodgeTimer_ = dodgeDuration_;
    dodgeCooldownTimer_ = dodgeCooldown_;

    // 回避開始からの経過時間を 0 にリセット（ここからカウントアップ）
    justEvasionTimer_ = 0.0f;
    hasJustEvaded_ = false;

    dodgeDirection_ = inputDir;
    // 正規化
    if (dodgeDirection_.x != 0.0f || dodgeDirection_.y != 0.0f) {
        float len = std::sqrt(dodgeDirection_.x * dodgeDirection_.x + dodgeDirection_.y * dodgeDirection_.y);
        dodgeDirection_.x /= len;
        dodgeDirection_.y /= len;
    }
}

bool PlayerEvasion::TryTriggerJustEvasion() {
    // 回避開始からの経過時間がウィンドウ内（＝回避し始めのタイミング）かつ未発動
    if (isDodging_ && justEvasionTimer_ <= justEvasionWindow_ && !hasJustEvaded_) {
        hasJustEvaded_ = true;
        return true;
    }
    return false;
}

void PlayerEvasion::SetJustEvasionWindow(float seconds) {
    justEvasionWindow_ = std::max(0.0f, seconds);
}

void PlayerEvasion::SetDodgeDuration(float seconds) {
    dodgeDuration_ = std::max(0.01f, seconds);
}

void PlayerEvasion::SetDodgeCooldown(float seconds) {
    dodgeCooldown_ = std::max(0.0f, seconds);
}

void PlayerEvasion::SetDodgeSpeed(float speed) {
    dodgeSpeed_ = std::max(0.0f, speed);
}
