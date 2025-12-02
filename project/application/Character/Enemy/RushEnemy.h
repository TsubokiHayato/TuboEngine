#pragma once
#include "Enemy.h"
#include <DirectXMath.h>

class Player; // 前方宣言 (詳細はcpp内でinclude)
class MapChipField;

// 突進専用エネミー: 射撃せず一定距離内で高速突進攻撃 + 準備(ため)時間
class RushEnemy : public Enemy {
public:
    RushEnemy() = default;
    ~RushEnemy() override = default;
    void Initialize() override; // 基底初期化 + パラメータ調整
    void Update() override;     // 射撃除去 + 突進行動
    void Draw() override;       // 弾描画なし
    void DrawImGui();           // パラメータ調整用

    // 調整用セッター
    void SetBaseMoveSpeed(float s) { baseMoveSpeed_ = s; moveSpeed_ = baseMoveSpeed_; }
    void SetRushSpeed(float s) { rushSpeed_ = s; }
    void SetRushTriggerDistance(float d) { rushTriggerDistance_ = d; }
    void SetRushDuration(float sec) { rushDuration_ = sec; }
    void SetTurnSpeed(float s) { turnSpeed_ = s; }
    void SetPrepareDuration(float sec) { prepareDuration_ = sec; }
    void SetPrepareMoveSpeed(float s) { prepareMoveSpeed_ = s; }
    void SetRushCooldown(float sec) { rushCooldownDuration_ = sec; }
    void SetExitHysteresis(float d) { exitHysteresis_ = d; }
    void SetStopDuration(float sec) { stopDuration_ = sec; }

private:
    // 追跡関連
    float baseMoveSpeed_ = 0.10f;      // 通常追跡速度
    // 突進関連
    float rushSpeed_ = 0.35f;          // 突進速度
    float rushTriggerDistance_ = 4.5f; // 突進開始距離
    float rushDuration_ = 0.6f;        // 突進持続秒
    bool  isPreparing_ = false;        // 突進前ため状態
    float prepareDuration_ = 0.4f;     // ため秒数
    float prepareTimer_ = 0.0f;        // 残りため時間
    float prepareMoveSpeed_ = 0.06f;   // ため中の前進速度(演出用 上げて停滞改善)
    bool  isRushing_ = false;          // 突進中フラグ
    float rushTimer_ = 0.0f;           // 残り突進時間
    // 停止
    bool  isStopping_ = false;         // 突進直後の停止中
    float stopDuration_ = 0.6f;        // 停止秒数
    float stopTimer_ = 0.0f;
    // クールダウン: 次の突進まで待機し再ポジション
    float rushCooldownDuration_ = 1.5f; // デフォルトのクールダウンを延長（連続防止）
    float rushCooldownTimer_ = 0.0f;
    // トリガー外へ一度出るまで次の突進を許可しないヒステリシス
    float exitHysteresis_ = 0.3f;      // しきいに加える余裕距離
    bool  requireExitBeforeNextRush_ = false; // 次突進前にトリガー外へ出る必要
    Vector3 rushDir_ {1.0f, 0.0f, 0.0f}; // 突進の固定進行方向

    static float NormalizeAngle(float angle);
    static void MoveWithCollision(Vector3& positionRef, const Vector3& desiredMove, MapChipField* field);
};
