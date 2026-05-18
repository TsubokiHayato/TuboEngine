#include "PlayerEvasion.h"
#include <cmath>

namespace {
    constexpr float kFixedDeltaTime = 1.0f / 60.0f;
}

PlayerEvasion::PlayerEvasion() {}

void PlayerEvasion::Update() {
    if (dodgeCooldownTimer_ > 0.0f) {
        dodgeCooldownTimer_ -= kFixedDeltaTime;
        if (dodgeCooldownTimer_ < 0.0f) {
            dodgeCooldownTimer_ = 0.0f;
        }
    }

    if (justEvasionTimer_ > 0.0f) {
        justEvasionTimer_ -= kFixedDeltaTime;
    }

    if (isDodging_) {
        dodgeTimer_ -= kFixedDeltaTime;
        if (dodgeTimer_ <= 0.0f) {
            isDodging_ = false;
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
    
    justEvasionTimer_ = justEvasionWindow_;
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
    if (isDodging_ && justEvasionTimer_ > 0.0f && !hasJustEvaded_) {
        hasJustEvaded_ = true;
        return true;
    }
    return false;
}
