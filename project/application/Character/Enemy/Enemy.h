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
	 * @return 視認中ならTrue。
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
	TuboEngine::Math::Vector3 GetPosition() const { return position_; }
	/** @brief 位置を設定します。 @param pos 位置。 */
	void SetPosition(const TuboEngine::Math::Vector3& pos) { position_ = pos; }
	/** @brief 回転を取得します。 @return 回転。 */
	TuboEngine::Math::Vector3 GetRotation() const { return rotation_; }
	/** @brief 回転を設定します。 @param rot 回転。 */
	void SetRotation(const TuboEngine::Math::Vector3& rot) { rotation_ = rot; }
	/** @brief スケールを取得します。 @return スケール。 */
	TuboEngine::Math::Vector3 GetScale() const { return scale_; }
	/** @brief スケールを設定します。 @param scl スケール。 */
	void SetScale(const TuboEngine::Math::Vector3& scl) { scale_ = scl; }
	/** @brief 生存状態を取得します。 @return 生存中ならTrue。 */
	bool GetIsAlive() const { return isAlive_; }
	/** @brief 生存状態を設定します。 @param alive 生存中ならTrue。 */
	void SetIsAlive(bool alive) { isAlive_ = alive; }
	/** @brief 操作対象のプレイヤー参照を設定します。 @param player プレイヤー。 */
	void SetPlayer(Player* player) { player_ = player; }
	/** @brief マップチップフィールド参照を設定します。 @param field マップチップフィールド。 */
	void SetMapChipField(MapChipField* field) { mapChipField_ = field; }
	/** @brief 現在HPを取得します。 @return HP。 */
	int GetHP() const { return hp_; }
	/** @brief 最大HPを取得します。 @return 最大HP。 */
	int GetMaxHP() const { return 10; }

	// 視野角・距離の取得/設定
	float GetViewAngleDeg() const { return viewAngleDeg_; }
	void SetViewAngleDeg(float deg) { viewAngleDeg_ = std::clamp(deg, 1.0f, 360.0f); }
	float GetViewDistance() const { return viewDistance_; }
	void SetViewDistance(float dist) { viewDistance_ = (dist < 0.0f) ? 0.0f : dist; }

	enum class State { Idle, Alert, LookAround, Patrol, Chase, Attack };

protected:
	TuboEngine::Math::Vector3 position_{};
	TuboEngine::Math::Vector3 rotation_{};
	TuboEngine::Math::Vector3 scale_ = {1.0f, 1.0f, 1.0f};
	TuboEngine::Math::Vector3 velocity_{};
	float turnSpeed_ = 0.1f;
	float moveSpeed_ = 0.08f;
	float shootDistance_ = 7.0f;
	float moveStartDistance_ = 15.0f;
	int hp_ = 100;
	bool isAlive_ = true;
	bool isHit_ = false;
	bool wasHit_ = false;
	std::unique_ptr<Object3d> object3d_;
	State state_ = State::Idle;
	float viewAngleDeg_ = 90.0f;
	float viewDistance_ = 15.0f;
	int viewLineDiv_ = 16;
	TuboEngine::Math::Vector4 viewColor_ = {1.0f, 1.0f, 0.0f, 0.7f};
	TuboEngine::Math::Vector3 lastSeenPlayerPos_ = {0.0f, 0.0f, 0.0f};
	float lastSeenTimer_ = 0.0f;
	float lastSeenDuration_ = 3.0f;
	std::unique_ptr<EnemyNormalBullet> bullet_;
	float lookAroundBaseAngle_ = 0.0f;
	float lookAroundTargetAngle_ = 0.0f;
	int lookAroundDirection_ = 1;
	float lookAroundAngleWidth_ = 1.25f;
	float lookAroundSpeed_ = 0.06f;
	int lookAroundCount_ = 0;
	int lookAroundMaxCount_ = 4;
	bool lookAroundInitialized_ = false;
	float idleLookAroundIntervalSec_ = 4.0f;
	float idleLookAroundTimer_ = 0.0f;
	enum class IdleBackPhase { None, ToBack, Hold, Return };
	IdleBackPhase idleBackPhase_ = IdleBackPhase::None;
	float idleBackTurnSpeed_ = 0.03f;
	float idleBackHoldSec_ = 0.5f;
	float idleBackHoldTimer_ = 0.0f;
	float idleBackStartAngle_ = 0.0f;
	float idleBackTargetAngle_ = 0.0f;
	bool showSurpriseIcon_ = false;
	TuboEngine::Math::Vector4 stateIconColor_ = {1, 0, 0, 1};
	float stateIconSize_ = 0.8f;
	float stateIconHeight_ = 3.0f;
	float stateIconLineWidth_ = 0.08f;
	float bulletTimer_ = 0.0f;
	bool wantShoot_ = false;
	std::vector<TuboEngine::Math::Vector3> currentPath_;
	size_t pathCursor_ = 0;
	int lastPathGoalIndex_ = -1;
	float waypointArriveEps_ = 0.15f;
	IParticleEmitter* hitEmitter_ = nullptr;
	IParticleEmitter* hitRingEmitter_ = nullptr;
	IParticleEmitter* deathEmitter_ = nullptr;
	bool deathEffectPlayed_ = false;
	Camera* camera_ = nullptr;
	MapChipField* mapChipField_ = nullptr;
	Player* player_ = nullptr;
	void ClearPath() {
		currentPath_.clear();
		pathCursor_ = 0;
		lastPathGoalIndex_ = -1;
	}
	bool BuildPathTo(const TuboEngine::Math::Vector3& worldGoal);

	// --- Update helpers (function length reduction) ---
	float GetFixedDeltaTime_() const { return 1.0f / 60.0f; }
	bool UpdateDeathIfNeeded_();
	void UpdatePerception_(float dt, bool& outCanSeePlayer, float& outDistanceToPlayer);
	void UpdateIcons_(float dt);
	void UpdateState_(bool canSeePlayer, float distanceToPlayer);
	void UpdateFacing_(bool canSeePlayer);
	void UpdateShooting_(float dt, bool canSeePlayer);
	void UpdateBehaviorByState_(float dt, bool canSeePlayer);
	void UpdateBullet_();
	void SyncToObject3d_();
	void SyncEmitters_();
	void UpdateHitAndResetFlags_();
	void UpdateIconSprites_();

	// --- Pathfinding helpers (BuildPathTo split) ---
	bool PreparePathfinding_(const TuboEngine::Math::Vector3& worldGoal, int& outW, int& outH, int& outSX, int& outSY, int& outGX, int& outGY);
	bool RunAStar_(int W, int H, int sx, int sy, int gFlat, std::vector<int>& outPathFlats);
	void BuildWorldPathFromFlats_(int W, const std::vector<int>& pathFlats);

	void ApplyKnockback(float dt);

protected:
	// 視認状態管理
	bool sawPlayerPrev_ = false;
	bool wasJustFound_ = false;
	bool wasJustLost_ = false;
	float surpriseTimer_ = 0.0f;
	float surpriseDuration_ = 0.8f;

	// アイコンスプライト
	std::unique_ptr<Sprite> questionIcon_;
	std::unique_ptr<Sprite> exclamationIcon_;
	float questionTimer_ = 0.0f;
	float exclamationTimer_ = 0.0f;
	float iconDuration_ = 1.2f;
	float iconOffsetY_ = -3.0f;
	float iconScreenOffsetY_ = 0.0f;
	TuboEngine::Math::Vector2 iconSize_ = {48.0f, 48.0f};

	bool showRaySamples_ = false;

	// --- Knockback（被弾時押し戻し）---
	float knockbackTimer_ = 0.0f;
	TuboEngine::Math::Vector3 knockbackVelocity_{0.0f, 0.0f, 0.0f};
	float knockbackStrength_ = 5.0f;
	float knockbackDamping_ = 0.85f;
};
