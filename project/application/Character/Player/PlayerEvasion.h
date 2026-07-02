#pragma once
#include "Vector3.h"

/// <summary>
/// プレイヤーの回避（ドッジ）動作と、ジャスト回避の受付判定・クールタイム管理を行うクラス。
/// </summary>
class PlayerEvasion {
public:
    /// <summary>
    /// コンストラクタ。
    /// </summary>
    PlayerEvasion();
    /// <summary>
    /// デストラクタ。
    /// </summary>
    ~PlayerEvasion() = default;

    /// <summary>
    /// 更新処理。
    /// </summary>
    void Update();
    /// <summary>
    /// 回避（ドッジ）を開始する。
    /// </summary>
    void StartDodge(const TuboEngine::Math::Vector3& inputDir);

    // --- 状態取得 ---
    bool IsDodging() const { return isDodging_; }
    // 回避開始からの経過時間がウィンドウ内かどうか
    bool IsJustEvasionWindow() const { return isDodging_ && justEvasionTimer_ <= justEvasionWindow_; }
    
    // デバッグ・ImGui用ゲッター
    float GetDodgeTimer() const { return dodgeTimer_; }
    float GetDodgeCooldownTimer() const { return dodgeCooldownTimer_; }
    float GetJustEvasionTimer() const { return justEvasionTimer_; }
    bool HasJustEvaded() const { return hasJustEvaded_; }
    float GetJustEvasionWindow() const { return justEvasionWindow_; }
    float GetDodgeDuration() const { return dodgeDuration_; }
    float GetDodgeCooldown() const { return dodgeCooldown_; }
    float GetDodgeSpeed() const { return dodgeSpeed_; }

    // ジャスト回避を試行する。成功した（受付時間内かつ未発動）場合は true を返す
    bool TryTriggerJustEvasion();

    // デバッグ・ImGui用セッター
    void SetJustEvasionWindow(float seconds);
    void SetDodgeDuration(float seconds);
    void SetDodgeCooldown(float seconds);
    void SetDodgeSpeed(float speed);

 /// <summary>
 /// DodgeDirection を取得する。
 /// </summary>
 TuboEngine::Math::Vector3 GetDodgeDirection() const { return dodgeDirection_; }

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
    // justEvasionWindow_: 回避開始から何秒以内に弾が当たればジャスト扱いか
    float justEvasionWindow_ = 3.00f;  // 
    // justEvasionTimer_: 回避開始からの経過時間（0からカウントアップ）
    float justEvasionTimer_ = 0.0f;
    bool hasJustEvaded_ = false;
};
