#pragma once
#include "Enemy.h"
#include <DirectXMath.h>

class Player; // 前方宣言 (詳細はcpp内でinclude)
class MapChipField;
class Collider;

// 突進専用エネミー: 射撃せず一定距離内で高速突進攻撃 + 準備(ため)時間
class RushEnemy : public Enemy {
public:
    RushEnemy() = default;
    ~RushEnemy() override = default;
    void Initialize() override; // 基底初期化 + パラメータ調整
    void Update() override;     // シーケンス制御
    void Draw() override;       // 弾描画なし
	void DrawSprite();
    void DrawImGui();           // パラメータ調整用
    void OnCollision(Collider* other) override; // プレイヤー接触時のノックバック & 追加リアクション



public:
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
    void SetLookAroundDuration(float sec) { lookAroundDuration_ = sec; }
    void SetShowDashPreview(bool f) { showDashPreview_ = f; }

private:
    // --- Parameters ---
    float baseMoveSpeed_ = 0.08f;      // 通常追跡速度 (少し遅く)
    float rushSpeed_ = 0.30f;          // 突進速度 (やや抑制)
    float rushTriggerDistance_ = 5.0f; // 突進開始距離
    float rushDuration_ = 0.8f;        // 突進持続秒 (少し延長)
    bool  isPreparing_ = false;        // 突進前ため状態
    float prepareDuration_ = 0.9f;     // ため秒数 (延長)
    float prepareTimer_ = 0.0f;        // 残りため時間
    float prepareMoveSpeed_ = 0.03f;   // ため中の前進速度 (遅く)
    bool  isRushing_ = false;          // 突進中フラグ
    float rushTimer_ = 0.0f;           // 残り突進時間
    // 突進開始時の一時的な伸び演出用タイマー
    float rushStretchTimer_ = 0.0f;      // 秒数
    float rushStretchDuration_ = 0.12f;  // 伸び演出の長さ

    // 停止
    bool  isStopping_ = false;         // 突進直後の停止中
    float stopDuration_ = 1.0f;        // 停止秒数 (延長)
    float stopTimer_ = 0.0f;
    // クールダウン
    float rushCooldownDuration_ = 2.5f; // クールダウン延長
    float rushCooldownTimer_ = 0.0f;
    // スタン（クールダウン中の硬直）
    bool  isStunned_ = false;
    float stunTimer_ = 0.0f;

    // ヒステリシス
    float exitHysteresis_ = 0.5f;      // 余裕距離増加
    bool  requireExitBeforeNextRush_ = false;
    // 見回し
    float lookAroundDuration_ = 1.8f;  // 見回し秒数
    float lookAroundTimer_ = 0.0f;
    bool  isScanning_ = false;         // 見回し中
    // 衝突リアクション
    bool  isReacting_ = false;         // 壁にぶつかったリアクション中
    float reactionDuration_ = 0.4f;    // リアクション秒数
    float reactionTimer_ = 0.0f;       // 残りリアクション時間
    float reactionBackoffSpeed_ = 0.25f; // ノックバック速度
    Vector3 reactionDir_ {0.0f, 0.0f, 0.0f}; // リアクション方向（反射方向）
    bool  endedRushWithoutWall_ = false; // 壁非ヒットで突進タイマー終了した直後フラグ

    // 直前のリアクションソース（振り向き制御用）
    enum class ReactionSource { None, Wall, Player };
    ReactionSource lastReactionSource_ = ReactionSource::None;

    // プレビュー表示
    bool  showDashPreview_ = true;     // チャージ中に突進到達予測を表示

    Vector3 rushDir_ {1.0f, 0.0f, 0.0f}; // 突進の固定進行方向

    // --- Helpers ---
    static float NormalizeAngle(float angle);
    static void MoveWithCollision(Vector3& positionRef, const Vector3& desiredMove, MapChipField* field);

    // Update helpers (readability)
    void UpdatePerceptionAndTimers(float dt, bool& canSeePlayer, float& distanceToPlayer);
    void UpdateStateByVision(bool canSeePlayer, float distanceToPlayer);
    void UpdateFacingWhenNeeded(bool canSeePlayer);
    void UpdateAttackState(float dt);
    void HandlePrepare(float dt);
    void HandleRushing(float dt);
    void HandleReacting(float dt);
    bool CheckWallHit(const Vector3& desiredMove, Vector3& outHitNormal) const;
    void ApplyChargeAndVisuals(float dt);
    void DrawDebugGizmos();

    // Collision helpers
    bool HandleReactingEarly(Collider* other, uint32_t typeID);
    bool HandlePlayerCollision(Collider* other);
    void HandleWeaponAfterRush(Collider* other, uint32_t typeID);
};
