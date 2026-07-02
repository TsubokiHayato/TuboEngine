#pragma once
#include <algorithm> // for std::clamp
#include "engine/BT/BehaviorTree.h"
#include "Character/BaseCharacter.h"
#include "Bullet/Enemy/EnemyNormalBullet.h"
#include "MapChip/MapChipField.h"
#include "Particle.h"
#include "ParticleEmitter.h"
#include"Sprite.h"
#include "Camera/FollowTopDownCamera.h"
// 演出用: 前方宣言のみで十分
class IParticleEmitter;

class Player;

/// <summary>
/// 敵キャラクターの基底クラス。ビヘイビアツリーによるAI・索敵・弾の発射・被弾/死亡演出などを行う。
/// </summary>
class Enemy : public BaseCharacter {
public:
    /// <summary>
    /// コンストラクタ。
    /// </summary>
    Enemy();
    /// <summary>
    /// デストラクタ。
    /// </summary>
    ~Enemy() override;
    /// <summary>
    /// 初期化処理。
    /// </summary>
    void Initialize() override;
    /// <summary>
    /// 更新処理。
    /// </summary>
    void Update() override;
    /// <summary>
    /// 描画処理。
    /// </summary>
    void Draw() override;
    /// <summary>
    /// パーティクル描画。
    /// </summary>
    void ParticleDraw();
    /// <summary>
    /// ImGuiによるデバッグ表示。
    /// </summary>
    void DrawImGui();
    /// <summary>
    /// 移動処理。
    /// </summary>
    void Move();
    /// <summary>
    /// プレイヤーを視認できるかどうかを判定する。
    /// </summary>
    bool CanSeePlayer();
    /// <summary>
    /// 視界（視野角）の可視化描画。
    /// </summary>
    void DrawViewCone();
    /// <summary>
    /// プレイヤーを最後に視認した位置のマーク描画。
    /// </summary>
    void DrawLastSeenMark();
    /// <summary>
    /// 現在のAIステートを示すアイコン描画。
    /// </summary>
    void DrawStateIcon();
    /// <summary>
    /// 被弾時のパーティクルを発生させる。
    /// </summary>
    void EmitHitParticle();
    /// <summary>
    /// 死亡時のパーティクルを発生させる。
    /// </summary>
    void EmitDeathParticle();
    /// <summary>
    /// 衝突時の処理。
    /// </summary>
    void OnCollision(Collider* other) override;
	/// <summary>
	/// 中心座標を取得する。
	/// </summary>
	TuboEngine::Math::Vector3 GetCenterPosition() const override;

    /// <summary>
    /// カメラを設定する。
    /// </summary>
    void SetCamera(TuboEngine::Camera* camera) { camera_ = camera; }
	/// <summary>
	/// 座標の取得・設定。
	/// </summary>
	TuboEngine::Math::Vector3 GetPosition() const { return position; }
	void SetPosition(const TuboEngine::Math::Vector3& pos) { position = pos; }
	/// <summary>
	/// 回転の取得・設定。
	/// </summary>
	TuboEngine::Math::Vector3 GetRotation() const { return rotation; }
	void SetRotation(const TuboEngine::Math::Vector3& rot) { rotation = rot; }
	/// <summary>
	/// スケールの取得・設定。
	/// </summary>
	TuboEngine::Math::Vector3 GetScale() const { return scale; }
	void SetScale(const TuboEngine::Math::Vector3& scl) { scale = scl; }
    /// <summary>
    /// 生存フラグの取得・設定。
    /// </summary>
    bool GetIsAlive() const { return isAlive; }
    void SetIsAlive(bool alive) { isAlive = alive; }
    /// <summary>
    /// プレイヤーの参照を設定する。
    /// </summary>
    void SetPlayer(Player* player) { player_ = player; }
    /// <summary>
    /// マップチップフィールドの参照を設定する。
    /// </summary>
    void SetMapChipField(MapChipField* field) { mapChipField = field; }
    /// <summary>
    /// FollowCamera を設定する。
    /// </summary>
    void SetFollowCamera(FollowTopDownCamera* camera) { followCamera_ = camera; }
    /// <summary>
    /// HPを取得する。
    /// </summary>
    int GetHP() const { return HP; }
    /// <summary>
    /// MaxHP を取得する。
    /// </summary>
    int GetMaxHP() const { return 10; }

    // 視野角・距離の取得/設定
    float GetViewAngleDeg() const { return kViewAngleDeg; }
    void SetViewAngleDeg(float deg) { kViewAngleDeg = std::clamp(deg, 1.0f, 360.0f); }
    float GetViewDistance() const { return kViewDistance; }
    void SetViewDistance(float dist) { kViewDistance = (dist < 0.0f) ? 0.0f : dist; }

    // 攻撃範囲（射撃エネミー用）
    float GetAttackRange() const { return attackRange_; }
    void SetAttackRange(float range) { attackRange_ = (range < 0.0f) ? 0.0f : range; }

    enum class State { Idle, Alert, LookAround, Patrol, Chase, Attack };

    // ---- ビヘイビアツリー ----
    // BuildBehaviorTree() を Initialize() 末尾で呼ぶことで bt_ を構築する。
    // サブクラスはオーバーライドして独自ツリーを持てる。
    virtual void BuildBehaviorTree();

    // サブクラスでオーバーライドしてモデルカラーを変える
    virtual TuboEngine::Math::Vector4 GetModelColor() const { return {1.0f, 0.0f, 0.0f, 1.0f}; }

protected: 
	TuboEngine::Math::Vector3 position;
	TuboEngine::Math::Vector3 rotation;
	TuboEngine::Math::Vector3 scale = {1.0f, 1.0f, 1.0f};
	TuboEngine::Math::Vector3 velocity;
    float turnSpeed_ = 0.1f;
    float moveSpeed_ = 0.08f;
	float shootDistance_ = 25.0f;
    float moveStartDistance_ = 50.0f;

    // 0 なら常に Chase しつつ射撃（遠距離射撃）。
    float attackRange_ = 25.0f;
    int HP = 100;
    bool isAlive = true;
    bool isHit = false;
    bool wasHit = false;
	std::unique_ptr<TuboEngine::Object3d> object3d;
    State state_ = State::Idle;
    float kViewAngleDeg = 90.0f;
    float kViewDistance = 50.0f;
    int kViewLineDiv = 8;
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
    IParticleEmitter* mistEmitter_ = nullptr;      // 霧散演出用ミスト
    bool isDying_ = false;                         // 死亡演出中か
    float deathTimer_ = 0.0f;                      // 死亡演出のタイマー
    static constexpr float kDeathDuration = 0.6f;  // 演出の長さ
    bool deathEffectPlayed_ = false;

    FollowTopDownCamera* followCamera_ = nullptr;

    /// <summary>
    /// 通常弾を使用するかどうか。
    /// </summary>
    virtual bool UseNormalBullet() const { return true; }

	TuboEngine::Camera* camera_ = nullptr;
    MapChipField* mapChipField = nullptr;
    Player* player_ = nullptr;
    /// <summary>
    /// Path を消去する。
    /// </summary>
    void ClearPath() { currentPath_.clear(); pathCursor_ = 0; lastPathGoalIndex_ = -1; }
	/// <summary>
	/// PathTo を構築する。
	/// </summary>
	bool BuildPathTo(const TuboEngine::Math::Vector3& worldGoal);

    // ---- BT 用フレームキャッシュ ----
    std::unique_ptr<BT::BehaviorNode> bt_;  ///< ビヘイビアツリーのルートノード
    bool  btCanSee_ = false;               ///< 今フレームのプレイヤー視認フラグ
    float btDist_   = 0.0f;               ///< 今フレームのプレイヤーまでの距離
    bool  btIsDoingLookAround_ = false;   ///< LookAround 継続中フラグ

    // 視認状態管理
    bool sawPlayerPrev_ = false;          // 前フレーム視認していたか
    bool wasJustFound_ = false;           // 今フレーム見つけた
    bool wasJustLost_ = false;            // 今フレーム見失った
    // 驚き演出（未視認攻撃時の同時表示維持用）
    float surpriseTimer_ = 0.0f;          // 残時間
    float surpriseDuration_ = 0.8f;       // 既定秒数

    // アイコンスプライト
	std::unique_ptr<TuboEngine::Sprite> questionIcon_;
	std::unique_ptr<TuboEngine::Sprite> exclamationIcon_;
    float questionTimer_ = 0.0f;          // Questionの表示残時間
    float exclamationTimer_ = 0.0f;       // Exclamationの表示残時間
    float iconDuration_ = 1.2f;           // デフォルト表示時間（秒）
    float iconOffsetY_ = -3.0f;            // 頭上へのワールドオフセット
    float iconScreenOffsetY_ = 0.0f;      // 追加のスクリーン座標Yオフセット（ピクセル、上に行くほど負）
	TuboEngine::Math::Vector2 iconSize_ = {48.0f, 48.0f}; // アイコンサイズ

    // レイサンプルデバッグ表示切替
    bool showRaySamples_ = false; // デフォルトOFF、ImGuiで切替

    // --- Hit Shake ---
    float hitShakeTimer_ = 0.0f;
    float hitShakeDuration_ = 0.15f;
    float hitShakeStrength_ = 0.3f;
    TuboEngine::Math::Vector3 hitShakeOffset_{0.0f, 0.0f, 0.0f};
    /// <summary>
    /// HitShake を適用する。
    /// </summary>
    void ApplyHitShake(float dt);

public:
    // アイコン制御API
    void ShowQuestion(float durationSec) { questionTimer_ = durationSec <= 0.0f ? iconDuration_ : durationSec; }
    void ShowExclamation(float durationSec) { exclamationTimer_ = durationSec <= 0.0f ? iconDuration_ : durationSec; }
    void ClearIcons() { questionTimer_ = 0.0f; exclamationTimer_ = 0.0f; surpriseTimer_ = 0.0f; }
};
