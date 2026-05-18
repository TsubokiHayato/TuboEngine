#pragma once
#include "Vector3.h"

class PlayerEvasion {
public:
    PlayerEvasion();
    ~PlayerEvasion() = default;

    void Update();
    void StartDodge(const TuboEngine::Math::Vector3& inputDir);

    // --- 状態取得 ---
    bool IsDodging() const { return isDodging_; }
    bool IsJustEvasionWindow() const { return justEvasionTimer_ > 0.0f; }
    
    // デバッグ・ImGui用ゲッター
    float GetDodgeTimer() const { return dodgeTimer_; }
    float GetDodgeCooldownTimer() const { return dodgeCooldownTimer_; }
    float GetJustEvasionTimer() const { return justEvasionTimer_; }
    bool HasJustEvaded() const { return hasJustEvaded_; }

    // ジャスト回避を試行する。成功した（受付時間内かつ未発動）場合は true を返す
    bool TryTriggerJustEvasion();

    TuboEngine::Math::Vector3 GetDodgeDirection() const { return dodgeDirection_; }
    float GetDodgeSpeed() const { return dodgeSpeed_; }

private:
    // 回避関連
    bool isDodging_ = false;
    float dodgeTimer_ = 0.0f;
    float dodgeDuration_ = 0.2f;
    float dodgeCooldownTimer_ = 0.0f;
    float dodgeCooldown_ = 1.0f;
    float dodgeSpeed_ = 0.5f;
    TuboEngine::Math::Vector3 dodgeDirection_{0.0f, 0.0f, 0.0f};

    // ジャスト回避関連
    float justEvasionWindow_ = 1.25f;
    float justEvasionTimer_ = 0.0f;
    bool hasJustEvaded_ = false;
};
