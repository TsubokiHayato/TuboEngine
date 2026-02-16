#pragma once
#include <algorithm> // for std::clamp
#include "Character/BaseCharacter.h"
#include "Bullet/Enemy/EnemyNormalBullet.h"
#include "MapChip/MapChipField.h"
#include "Particle.h"
#include "ParticleEmitter.h"
// 演出用: 前方宣言のみで十分
class IParticleEmitter;

class Player;
class Sprite; // スプライト前方宣言

/**
 * @brief 敵キャラクタークラス。
 *
 * @details
 * 本クラスの責務は、敵キャラクターの
 * - 行動状態（待機/巡回/追跡/攻撃など）の更新
 * - プレイヤー視認判定（視野角・距離）
 * - 移動/旋回/射撃の制御
 * - 被弾/死亡などの状態管理と演出（パーティクル、アイコン）
 * - 当たり判定（`Collider`）との連携
 * をまとめて管理することです。
 */
class Enemy : public BaseCharacter {
public:
    /** @brief コンストラクタ。 */
    Enemy();
    /** @brief デストラクタ。 */
    ~Enemy() override;

    /** @brief 初期化処理。モデル/状態/各種タイマーを初期化します。 */
    void Initialize() override;
    /** @brief 更新処理。状態遷移・移動・攻撃・演出の更新を行います。 */
    void Update() override;
    /** @brief 描画処理。3Dモデルや補助表示を描画します。 */
    void Draw() override;

    /** @brief パーティクル描画（必要な場合のみ分離）。 */
    void ParticleDraw();
    /** @brief ImGui描画（デバッグ用）。 */
    void DrawImGui();

    /** @brief 移動処理（内部状態に基づく移動）。 */
    void Move();

    /**
     * @brief プレイヤーを視認できているか判定します。
     * @return 視認中ならtrue。
     */
    bool CanSeePlayer();

    /** @brief 視野範囲の可視化（デバッグ用）。 */
    void DrawViewCone();
    /** @brief 最後に見た位置のマーク描画（デバッグ用）。 */
    void DrawLastSeenMark();
    /** @brief 状態アイコン描画（疑問符/！など）。 */
    void DrawStateIcon();

    /** @brief 被弾パーティクルの発生。 */
    void EmitHitParticle();
    /** @brief 死亡パーティクルの発生。 */
    void EmitDeathParticle();

    /**
     * @brief 衝突時処理。
     * @param other 衝突相手のコライダー。
     */
    void OnCollision(Collider* other) override;

	/**
	 * @brief 当たり判定の中心座標を取得します。
	 * @return ワールド空間での中心座標。
	 */
	TuboEngine::Math::Vector3 GetCenterPosition() const override;

    /** @brief 使用するカメラを設定します。 @param camera カメラ。 */
    void SetCamera(Camera* camera) { camera_ = camera; }
	/** @brief 位置を取得します。 @return 位置。 */
	TuboEngine::Math::Vector3 GetPosition() const { return position; }
	/** @brief 位置を設定します。 @param pos 位置。 */
	void SetPosition(const TuboEngine::Math::Vector3& pos) { position = pos; }
	/** @brief 回転を取得します。 @return 回転。 */
	TuboEngine::Math::Vector3 GetRotation() const { return rotation; }
	/** @brief 回転を設定します。 @param rot 回転。 */
	void SetRotation(const TuboEngine::Math::Vector3& rot) { rotation = rot; }
	/** @brief スケールを取得します。 @return スケール。 */
	TuboEngine::Math::Vector3 GetScale() const { return scale; }
	/** @brief スケールを設定します。 @param scl スケール。 */
	void SetScale(const TuboEngine::Math::Vector3& scl) { scale = scl; }
    /** @brief 生存状態を取得します。 @return 生存中ならtrue。 */
    bool GetIsAlive() const { return isAlive; }
    /** @brief 生存状態を設定します。 @param alive 生存中ならtrue。 */
    void SetIsAlive(bool alive) { isAlive = alive; }
    /** @brief 操作対象のプレイヤー参照を設定します。 @param player プレイヤー。 */
    void SetPlayer(Player* player) { player_ = player; }
    /** @brief マップチップフィールド参照を設定します。 @param field マップチップフィールド。 */
    void SetMapChipField(MapChipField* field) { mapChipField = field; }
    /** @brief 現在HPを取得します。 @return HP。 */
    int GetHP() const { return HP; }
    /** @brief 最大HPを取得します。 @return 最大HP。 */
    int GetMaxHP() const { return 10; }

    // 視野角・距離の取得/設定
    float GetViewAngleDeg() const { return kViewAngleDeg; }
    void SetViewAngleDeg(float deg) { kViewAngleDeg = std::clamp(deg, 1.0f, 360.0f); }
    float GetViewDistance() const { return kViewDistance; }
    void SetViewDistance(float dist) { kViewDistance = (dist < 0.0f) ? 0.0f : dist; }

    enum class State { Idle, Alert, LookAround, Patrol, Chase, Attack };

protected: 
	TuboEngine::Math::Vector3 position;
	TuboEngine::Math::Vector3 rotation;
	TuboEngine::Math::Vector3 scale = {1.0f, 1.0f, 1.0f};
	TuboEngine::Math::Vector3 velocity;
    float turnSpeed_ = 0.1f;
    float moveSpeed_ = 0.08f;
    float shootDistance_ = 7.0f;
    float moveStartDistance_ = 15.0f;
    int HP = 100;
    bool isAlive = true;
    bool isHit = false;
    bool wasHit = false;
    std::unique_ptr<Object3d> object3d;
    State state_ = State::Idle;
    float kViewAngleDeg = 90.0f;
    float kViewDistance = 15.0f;
    int kViewLineDiv = 16;
	TuboEngine::Math::Vector4 kViewColor = {1.0f, 1.0f, 0.0f, 0.7f};
	TuboEngine::Math::Vector3 lastSeenPlayerPos = {0.0f, 0.0f, 0.0f};
    float lastSeenTimer = 0.0f;
    float kLastSeenDuration = 3.0f;
    std::unique_ptr<EnemyNormalBullet> bullet;
    float lookAroundBaseAngle = 0.0f;
    float lookAroundTargetAngle = 0.0f;
    int lookAroundDirection = 1;
    float lookAroundAngleWidth = 1.25f;
    float lookAroundSpeed = 0.06f;
    int lookAroundCount = 0;
    int lookAroundMaxCount = 4;
    bool lookAroundInitialized = false;
    float idleLookAroundIntervalSec = 4.0f;
    float idleLookAroundTimer = 0.0f;
    enum class IdleBackPhase { None, ToBack, Hold, Return };
    IdleBackPhase idleBackPhase_ = IdleBackPhase::None;
    float idleBackTurnSpeed = 0.03f;
    float idleBackHoldSec = 0.5f;
    float idleBackHoldTimer = 0.0f;
    float idleBackStartAngle = 0.0f;
    float idleBackTargetAngle = 0.0f;
    bool showSurpriseIcon_ = false;
	TuboEngine::Math::Vector4 stateIconColor = {1, 0, 0, 1};
    float stateIconSize = 0.8f;
    float stateIconHeight = 3.0f;
    float stateIconLineWidth = 0.08f;
    float bulletTimer_ = 0.0f;
    bool wantShoot_ = false;
	std::vector<TuboEngine::Math::Vector3> currentPath_;
    size_t pathCursor_ = 0;
    int lastPathGoalIndex_ = -1;
    float waypointArriveEps_ = 0.15f;
    IParticleEmitter* hitEmitter_ = nullptr;       // 既存: スパーク等
    IParticleEmitter* hitRingEmitter_ = nullptr;   // 追加: ヒット時の小リング
    IParticleEmitter* deathEmitter_ = nullptr;
    bool deathEffectPlayed_ = false;
    Camera* camera_ = nullptr;
    MapChipField* mapChipField = nullptr;
    Player* player_ = nullptr;
    void ClearPath() { currentPath_.clear(); pathCursor_ = 0; lastPathGoalIndex_ = -1; }
	bool BuildPathTo(const TuboEngine::Math::Vector3& worldGoal);

    // 視認状態管理
    bool sawPlayerPrev_ = false;          // 前フレーム視認していたか
    bool wasJustFound_ = false;           // 今フレーム見つけた
    bool wasJustLost_ = false;            // 今フレーム見失った
    // 驚き演出（未視認攻撃時の同時表示維持用）
    float surpriseTimer_ = 0.0f;          // 残時間
    float surpriseDuration_ = 0.8f;       // 既定秒数

    // アイコンスプライト
    std::unique_ptr<Sprite> questionIcon_;
    std::unique_ptr<Sprite> exclamationIcon_;
    float questionTimer_ = 0.0f;          // Questionの表示残時間
    float exclamationTimer_ = 0.0f;       // Exclamationの表示残時間
    float iconDuration_ = 1.2f;           // デフォルト表示時間（秒）
    float iconOffsetY_ = -3.0f;            // 頭上へのワールドオフセット
    float iconScreenOffsetY_ = 0.0f;      // 追加のスクリーン座標Yオフセット（ピクセル、上に行くほど負）
	TuboEngine::Math::Vector2 iconSize_ = {48.0f, 48.0f}; // アイコンサイズ

    // レイサンプルデバッグ表示切替
    bool showRaySamples_ = false; // デフォルトOFF、ImGuiで切替

    // --- Knockback（被弾時押し戻し）---
    float knockbackTimer_ = 0.0f;
	TuboEngine::Math::Vector3 knockbackVelocity_{0.0f, 0.0f, 0.0f};
    float knockbackStrength_ = 5.0f; // タイル幅の約5.0倍/秒
    float knockbackDamping_ = 0.85f;
    void ApplyKnockback(float dt);

public:
    // アイコン制御API
    void ShowQuestion(float durationSec) { questionTimer_ = durationSec <= 0.0f ? iconDuration_ : durationSec; }
    void ShowExclamation(float durationSec) { exclamationTimer_ = durationSec <= 0.0f ? iconDuration_ : durationSec; }
    void ClearIcons() { questionTimer_ = 0.0f; exclamationTimer_ = 0.0f; surpriseTimer_ = 0.0f; }
};
