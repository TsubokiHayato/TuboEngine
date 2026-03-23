#pragma once
#include "Character/BaseCharacter.h"
#include "Bullet/Player/PlayerBullet.h"
#include "Object3d.h"
#include "Sprite.h"
#include "MapChip/MapChipField.h"
#include "engine/graphic/Particle/ParticleManager.h"
#include "engine/graphic/Particle/Effects/Ring/RingEmitter.h"
#include "Camera.h"
#include "PlayerAutoController.h" // 追加
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
	// 見た目だけ更新（ゲームロジックを一切実行しない。Transition等の外部制御用）
	void UpdateVisualOnly();
	// 描画処理のオーバーライド
	void Draw() override;
	// 衝突時の処理のオーバーライド
	void OnCollision(Collider* other) override;
	// 当たり判定の中心座標を取得のオーバーライド
	TuboEngine::Math::Vector3 GetCenterPosition() const override;

	// ImGuiの描画処理
	void DrawImGui();

	// 弾を撃つ処理
	void Shoot();

	// 移動処理
	void Move();

	void Rotate();

	void ReticleDraw();

	void TriggerDashRing();

	// 斜め視点でもレティクル通りに飛ばすための方向取得関数（地面へレイキャスト）
	TuboEngine::Math::Vector3 GetAimDirectionFromReticle() const;


private:
	// --- 回避関連 ---
	void StartDodge();
	void UpdateDodge();
	TuboEngine::Math::Vector3 GetDodgeInputDirection() const;

public:
	///-----------------------------------
	///				ゲッター
	///------------------------------------

	// プレイヤーの位置を取得
	TuboEngine::Math::Vector3 GetPosition() const { return position; }
	// プレイヤーの回転を取得
	TuboEngine::Math::Vector3 GetRotation() const { return rotation; }
	// プレイヤーのスケールを取得
	TuboEngine::Math::Vector3 GetScale() const { return scale; }
	// プレイヤーの速度を取得
	TuboEngine::Math::Vector3 GetVelocity() const { return velocity; }
	// プレイヤーのHPを取得
	int GetHP() const { return HP; }
	// プレイヤーの死亡状態を取得
	bool GetIsAlive() const { return isAlive; }
	// プレイヤーの弾のリストを取得
	const std::vector<std::unique_ptr<PlayerBullet>>& GetBullets() const { return bullets; }
	bool IsDashing() const { return isDashing_; } // 既存なら流用、無ければダミー
	bool GetIsHit() const { return isHit; } // 被弾フラグのゲッター

	///-----------------------------------
	///				セッター
	///-------------------------------------

	// プレイヤーの位置を設定
	void SetPosition(const TuboEngine::Math::Vector3& position) { this->position = position; }
	// プレイヤーの回転を設定
	void SetRotation(const TuboEngine::Math::Vector3& rotation) { this->rotation = rotation; }
	// プレイヤーのスケールを設定
	void SetScale(const TuboEngine::Math::Vector3& scale) { this->scale = scale; }
	// プレイヤーの速度を設定
	void SetVelocity(const TuboEngine::Math::Vector3& velocity) { this->velocity = velocity; }
	// プレイヤーのHPを設定
	void SetHP(int HP) { this->HP = HP; }
	// プレイヤーの死亡状態を設定
	void SetIsDead(bool isAlive) { this->isAlive = isAlive; }
	// カメラを設定
	void SetCamera(TuboEngine::Camera* camera) {
		object3d->SetCamera(camera);
		camera_ = camera;
	}
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
	// 現在紐づいているマップチップフィールドを取得
	MapChipField* GetMapChipField() const { return mapChipField; }

	void SetMovementLocked(bool flag) { isMovementLocked = flag; }

	// 自動操作を有効/無効
	void SetAutoControlEnabled(bool enabled) { autoController_.SetEnabled(enabled); }
	bool IsAutoControlEnabled() const { return autoController_.IsEnabled(); }

    // 自動操作用: 敵リストをコントローラに渡す
    void SetEnemyList(const std::vector<Enemy*>& enemies) { autoController_.SetEnemyList(enemies); }

	// 自動操作用インターフェース（AutoControllerから呼ばれる）
	void SetAutoMoveDirection(const TuboEngine::Math::Vector3& dir) { autoMoveDir_ = dir; }
	void SetAutoShoot(bool enabled) { autoShoot_ = enabled; }

	// 近くの敵への向き（XY平面の正規化ベクトル）を設定
	void SetAutoAimDirection(const TuboEngine::Math::Vector3& dir) { autoAimDir_ = dir; }
	// 自動操作で回避開始
	void AutoStartDodge() { StartDodge(); }

	// --- 回避可能か ---
	bool CanDodge() const { return !isDodging && dodgeCooldownTimer <= 0.0f; }

private:
	///--------------------------------------------------
	///				引き渡し用変数
	///--------------------------------------------------
	MapChipField* mapChipField = nullptr; // マップチップフィールド

private:
	///--------------------------------------------------
	///			メンバ変数
	///--------------------------------------------------

	std::unique_ptr<TuboEngine::Object3d> object3d;     // 3Dオブジェクト
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

	TuboEngine::Math::Vector3 dodgeDirection = {0.0f, 0.0f, 0.0f};        // 回避方向

	TuboEngine::Math::Vector3 position; // プレイヤーの位置
	TuboEngine::Math::Vector3 rotation; // プレイヤーの回転
	TuboEngine::Math::Vector3 scale;    // プレイヤーのスケール

	TuboEngine::Math::Vector3 velocity; // プレイヤーの速度
	int HP;           // プレイヤーのHP
	bool isHit;       // プレイヤーがヒットしたかどうか
	bool isAlive;      // プレイヤーの死亡状態

	//Reticle

	std::unique_ptr<TuboEngine::Sprite> reticleSprite;        // スプライト
	TuboEngine::Math::Vector2 reticlePosition = {0.0f, 0.0f}; // レティクルの位置（画面中央）
	TuboEngine::Math::Vector2 reticleSize = {50.0f, 50.0f};   // レティクルのサイズ

	bool isMovementLocked=false;

	// --- 追加: 移動軌跡用パーティクルエミッター ---
	IParticleEmitter* trailEmitter_ = nullptr; // ParticleManager生成管理。解放はマネージャに委譲
	TuboEngine::Math::Vector3 prevPositionTrail_{};              // 前フレーム位置
	IParticleEmitter* dashRingEmitter_ = nullptr;
	bool wasDashingPrev_ = false;
	bool isDashing_ = false; // 既存のダッシュ状態に置き換え可
	TuboEngine::Camera* camera_ = nullptr; // 位置/方向参照用
	float dashRingOffsetForward_ = 0.0f; // カメラ前方方向へのオフセット量

	// 連続リング発生のためのタイマーと間隔
	float dodgeRingIntervalSec_ = 0.12f; // 回避中の連続発生間隔
	float dodgeRingEmitTimer_ = 0.0f;    // 次発生までの残り時間

	// Dash演出（ポストエフェクト）
	float dashPostEffectTimer_ = 0.0f;
	float dashPostEffectDuration_ = 0.25f;
	float dashRadialBlurPower_ = 0.06f; // 0.02がデフォルトなので少し強め

	PlayerAutoController autoController_; // 自動操作用

private:
	TuboEngine::Math::Vector3 autoMoveDir_{0.0f, 0.0f, 0.0f};
	bool autoShoot_ = false;

	//オートエイム用の方向（XY平面の正規化ベクトル）
	TuboEngine::Math::Vector3 autoAimDir_{0.0f, -1.0f, 0.0f};
};
