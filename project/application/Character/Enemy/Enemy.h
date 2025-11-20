#pragma once
#include "Character/BaseCharacter.h"
#include "Bullet/Enemy/EnemyNormalBullet.h"
#include "MapChip/MapChipField.h"
#include "Particle.h"
#include "ParticleEmitter.h"

///---------------------------------------------------
///				前方宣言
///---------------------------------------------------
class Player;
class Sprite; // スプライト前方宣言

class Enemy : public BaseCharacter {
	///---------------------------------------
	///				メンバ関数
	///-----------------------------------------
public:
	Enemy();
	~Enemy() override;

	void Initialize() override;
	void Update() override;
	void Draw() override;
	void ParticleDraw();
	void DrawImGui();

	void Move();

	// プレイヤーの方向を向く
	bool CanSeePlayer();
	// 視野扇形の描画
	void DrawViewCone();

	void DrawLastSeenMark();

	void DrawStateIcon();
	//void DrawExclamationMark(const Vector3& pos, float size, const Vector4& color, float width);
	//void DrawQuestionMark(const Vector3& pos, float size, const Vector4& color, float width);

	// ヒット演出のトリガー
	void EmitHitParticle();

	// 衝突時の処理のオーバーライド
	void OnCollision(Collider* other) override;

	// 当たり判定の中心座標を取得のオーバーライド
	Vector3 GetCenterPosition() const override;

	// 必要に応じてEnemy固有のメンバ変数・関数を追加

	///------------------------------------------------------
	///				ゲッター&セッター
	///------------------------------------------------------
public:
	// カメラ
	void SetCamera(Camera* camera) { camera_ = camera; }

	// 座標
	Vector3 GetPosition() const { return position; }
	void SetPosition(const Vector3& pos) { position = pos; }

	// 回転
	Vector3 GetRotation() const { return rotation; }
	void SetRotation(const Vector3& rot) { rotation = rot; }

	// スケール
	Vector3 GetScale() const { return scale; }
	void SetScale(const Vector3& scl) { scale = scl; }

	// 生存フラグ
	bool GetIsAllive() const { return isAllive; }
	void SetIsAlive(bool alive) { isAllive = alive; }

	void SetPlayer(Player* player) { player_ = player; }

	// マップチップフィールド
	void SetMapChipField(MapChipField* field) { mapChipField = field; }

	// 既存のゲッター・セッターの後に追加
	int GetHP() const { return HP; }
	int GetMaxHP() const { return 10; } // 既定値（必要に応じて調整）

	///----------------------------------
	///				受取り変数
	/// ---------------------------------
private:
	Camera* camera_ = nullptr; // カメラ

	///---------------------------------------
	///				メンバ変数
	///---------------------------------------
public:
	enum class State {
		Idle,      // 非発見状態（待機）
		Alert,     // 警戒状態（見失い地点へ移動）
		LookAround, // 見回し状態（警戒終了後も見回す）
		Patrol,    // 巡回モード
		Chase,     // 発見状態（追跡）
		Attack,    // 攻撃
	};

private:
	///-------トランスフォーム------///
	Vector3 position;
	Vector3 rotation;
	Vector3 scale = {1.0f, 1.0f, 1.0f};

	Vector3 velocity;
	float turnSpeed_ = 0.1f;
	float moveSpeed_ = 0.08f;
	float shootDistance_ = 7.0f;
	float moveStartDistance_ = 15.0f;

	int HP = 100;
	bool isAllive = true;
	bool isHit = false;
	bool wasHit = false;

	std::unique_ptr<Object3d> object3d;
	State state_ = State::Idle;

	///--視野--///
	float kViewAngleDeg = 90.0f;
	float kViewDistance = 15.0f;
	int kViewLineDiv = 16;
	Vector4 kViewColor = {1.0f, 1.0f, 0.0f, 0.7f};
	Vector3 lastSeenPlayerPos = {0.0f, 0.0f, 0.0f};
	float lastSeenTimer = 0.0f;
	float kLastSeenDuration = 3.0f;

	///-----Bullet-----///
	std::unique_ptr<EnemyNormalBullet> bullet;

	///-----Particle-----///
	/*std::unique_ptr<Particle> particle;
	std::unique_ptr<ParticleEmitter> particleEmitter_;

	Transform particleTranslate;
	Vector3 particleVelocity = {0.0f, 0.0f, 0.0f};
	Vector4 particleColor = {1.0f, 1.0f, 1.0f, 1.0f};
	float particleLifeTime = 1.0f;
	float particleCurrentTime = 0.0f;*/

	///-----見回し（LookAround状態用）-----///
	float lookAroundBaseAngle = 0.0f;
	float lookAroundTargetAngle = 0.0f;
	int lookAroundDirection = 1;          // 1:右, -1:左
	float lookAroundAngleWidth = 1.25f;   // ラジアン（約70度）
	float lookAroundSpeed = 0.06f;        // ラジアン/フレーム
	int lookAroundCount = 0;
	int lookAroundMaxCount = 4;
	bool lookAroundInitialized = false;

	///----- Idle: 一定間隔で後ろを向く -----///
	// 発動間隔（Idleでの背面確認の開始間隔）
	float idleLookAroundIntervalSec = 4.0f;
	float idleLookAroundTimer = 0.0f;

	// フェーズ制御
	enum class IdleBackPhase { None, ToBack, Hold, Return };
	IdleBackPhase idleBackPhase_ = IdleBackPhase::None;

	// 旋回・保持パラメータ
	float idleBackTurnSpeed = 0.03f;   // ラジアン/フレーム（ゆっくり）
	float idleBackHoldSec = 0.5f;      // 背面での保持秒数
	float idleBackHoldTimer = 0.0f;
	float idleBackStartAngle = 0.0f;   // 開始時の角度（戻り先）
	float idleBackTargetAngle = 0.0f;  // 背面の角度（start + π）

	bool showSurpriseIcon_ = false;

	Vector4 stateIconColor = {1, 0, 0, 1};
	float stateIconSize = 0.8f;
	float stateIconHeight = 3.0f;
	float stateIconLineWidth = 0.08f;

	// 射撃クールダウン
	float bulletTimer_ = 0.0f;
	bool wantShoot_ = false;

private:
	///----- 経路追従（タイル対応） -----///
	std::vector<Vector3> currentPath_;
	size_t pathCursor_ = 0;
	int lastPathGoalIndex_ = -1;
	float waypointArriveEps_ = 0.15f;

	void ClearPath() { currentPath_.clear(); pathCursor_ = 0; lastPathGoalIndex_ = -1; }
	bool BuildPathTo(const Vector3& worldGoal);
};
