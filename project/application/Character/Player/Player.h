#pragma once
#include "Character/BaseCharacter.h"
#include "Bullet/Player/PlayerBullet.h"
#include "Object3d.h"
#include "Sprite.h"
#include "MapChip/MapChipField.h"
#include "engine/graphic/Particle/ParticleManager.h"
#include "engine/graphic/Particle/RingEmitter.h"
#include "Camera.h"
// 前方宣言（ヘッダ依存軽減）
class IParticleEmitter;
///--------------------------------------------------
// プレイヤークラス
///--------------------------------------------------
class Player : public BaseCharacter {
public:
	///--------------------------------------------------
	///				メンバ関数
	///--------------------------------------------------

	// コンストラクタ
	Player();
	// デストラクタ
	~Player() override;

	// 初期化のオーバーライド
	void Initialize() override;
	// 更新処理のオーバーライド
	void Update() override;
	// 描画処理のオーバーライド
	void Draw() override;
	// 衝突時の処理のオーバーライド
	void OnCollision(Collider* other) override;
	// 当たり判定の中心座標を取得のオーバーライド
	Vector3 GetCenterPosition() const override;

	// ImGuiの描画処理
	void DrawImGui();

	// 弾を撃つ処理
	void Shoot();

	// 移動処理
	void Move();

	void Rotate();

	void ReticleDraw();

	void TriggerDashRing();

	

private:
	// --- 回避関連 ---
	void StartDodge();
	void UpdateDodge();
	bool CanDodge() const;
	Vector3 GetDodgeInputDirection() const;

public:
	///-----------------------------------
	///				ゲッター
	///------------------------------------

	// プレイヤーの位置を取得
	Vector3 GetPosition() const { return position; }
	// プレイヤーの回転を取得
	Vector3 GetRotation() const { return rotation; }
	// プレイヤーのスケールを取得
	Vector3 GetScale() const { return scale; }
	// プレイヤーの速度を取得
	Vector3 GetVelocity() const { return velocity; }
	// プレイヤーのHPを取得
	int GetHP() const { return HP; }
	// プレイヤーの死亡状態を取得
	bool GetIsAllive() const { return isAllive; }
	// プレイヤーの弾のリストを取得
	const std::vector<std::unique_ptr<PlayerBullet>>& GetBullets() const { return bullets; }
	bool IsDashing() const { return isDashing_; } // 既存なら流用、無ければダミー

	///-----------------------------------
	///				セッター
	///-------------------------------------

	// プレイヤーの位置を設定
	void SetPosition(const Vector3& position) { this->position = position; }
	// プレイヤーの回転を設定
	void SetRotation(const Vector3& rotation) { this->rotation = rotation; }
	// プレイヤーのスケールを設定
	void SetScale(const Vector3& scale) { this->scale = scale; }
	// プレイヤーの速度を設定
	void SetVelocity(const Vector3& velocity) { this->velocity = velocity; }
	// プレイヤーのHPを設定
	void SetHP(int HP) { this->HP = HP; }
	// プレイヤーの死亡状態を設定
	void SetIsDead(bool isAllive) { this->isAllive = isAllive; }
	// カメラを設定
	void SetCamera(Camera* camera) { object3d->SetCamera(camera); }
	void SetDashRingOffset(float forward) { dashRingOffsetForward_ = forward; }

	// モデルのアルファ設定
	void SetModelAlpha(float alpha) {
		Vector4 color = object3d->GetModelColor();
		color.w = alpha;
		object3d->SetModelColor(color);
	}

	// 環境マップ設定
	void CubeMapSet(const std::string& filePath) { object3d->SetCubeMapFilePath(filePath); }

	// マップチップフィールドを設定
	void SetMapChipField(MapChipField* mapChipField) { this->mapChipField = mapChipField; }

	void SetIsDontMove(bool flag) { isDontMove = flag; }

private:
	///--------------------------------------------------
	///				引き渡し用変数
	///--------------------------------------------------
	MapChipField* mapChipField = nullptr; // マップチップフィールド

private:
	///--------------------------------------------------
	///			メンバ変数
	///--------------------------------------------------

	std::unique_ptr<Object3d> object3d;                 // 3Dオブジェクト
	std::vector<std::unique_ptr<PlayerBullet>> bullets; // プレイヤーの弾のリスト
	float bulletTimer = 0.0f;                           // 発射間隔タイマー
	float cooldownTime = 0.2f;                          // クールダウン時間（秒）

	float damageCooldownTimer = 0.0f;                   // ダメージクールダウンタイマー
	float damageCooldownTime = 1.0f;                    // ダメージクールダウン時間（秒）

	// 回避行動
	bool isDodging = false;                             // 回避中フラグ
	float dodgeTimer = 0.0f;                            // 回避残り時間
	float dodgeCooldownTimer = 0.0f;                    // 回避クールダウンタイマー
	float dodgeDuration = 0.2f;                         // 回避時間（秒）
	float dodgeCooldown = 1.0f;                         // 回避クールダウン（秒）
	float dodgeSpeed = 0.5f;                            // 回避速度

	Vector3 dodgeDirection = {0.0f, 0.0f, 0.0f};        // 回避方向

	Vector3 position; // プレイヤーの位置
	Vector3 rotation; // プレイヤーの回転
	Vector3 scale;    // プレイヤーのスケール

	Vector3 velocity; // プレイヤーの速度
	int HP;           // プレイヤーのHP
	bool isHit;       // プレイヤーがヒットしたかどうか
	bool isAllive;      // プレイヤーの死亡状態

	//Reticle

	std::unique_ptr<Sprite> reticleSprite; // スプライト
	Vector2 reticlePosition = {0.0f, 0.0f}; // レティクルの位置（画面中央）
	Vector2 reticleSize = {50.0f, 50.0f};   // レティクルのサイズ

	bool isDontMove=false;

	// --- 追加: 移動軌跡用パーティクルエミッター ---
	IParticleEmitter* trailEmitter_ = nullptr; // ParticleManager生成管理。解放はマネージャに委譲
	Vector3 prevPositionTrail_{};              // 前フレーム位置
	IParticleEmitter* dashRingEmitter_ = nullptr;
	bool wasDashingPrev_ = false;
	bool isDashing_ = false; // 既存のダッシュ状態に置き換え可
	Camera* camera_ = nullptr; // 位置/方向参照用
	float dashRingOffsetForward_ = 0.0f; // カメラ前方方向へのオフセット量

	// 連続リング発生のためのタイマーと間隔
	float dodgeRingIntervalSec_ = 0.12f; // 回避中の連続発生間隔
	float dodgeRingEmitTimer_ = 0.0f;    // 次発生までの残り時間
};
