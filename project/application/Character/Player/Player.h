#pragma once
#include "Character/BaseCharacter.h"
#include "Bullet/Player/PlayerBullet.h"
#include "Object3d.h"
#include "Sprite.h"
#include "MapChip/MapChipField.h"
#include "engine/graphic/Particle/ParticleManager.h"
#include "engine/graphic/Particle/Effects/Ring/RingEmitter.h"
#include "Camera.h"
// 前方宣言（ヘッダ依存軽減）
class IParticleEmitter;

///--------------------------------------------------
/// @brief プレイヤークラス。
///
/// @details
/// 本クラスの責務は、プレイヤーキャラクターの
/// - 入力に基づく移動/回転/回避（ダッシュ）
/// - 弾発射などの攻撃処理
/// - 被弾/無敵時間などの状態管理
/// - 3D/2D描画（モデル/レティクル）
/// - 当たり判定（`Collider`）との連携
/// をまとめて管理することです。
///
/// @note 実際の入力取得や演出は実装側（`.cpp`）で行い、本ヘッダは公開APIを定義します。
///--------------------------------------------------
class Player : public BaseCharacter {
public:
	///--------------------------------------------------
	///				メンバ関数
	///--------------------------------------------------

	/** @brief コンストラクタ。 */
	Player();
	/** @brief デストラクタ。 */
	~Player() override;

	/**
	 * @brief プレイヤーの初期化。
	 * @details モデル/当たり判定/各種タイマーやパラメータを初期化します。
	 */
	void Initialize() override;
	/**
	 * @brief プレイヤーの更新。
	 * @details 入力に応じた移動・回避・射撃、各種クールダウンの更新を行います。
	 */
	void Update() override;
	/**
	 * @brief プレイヤーの描画。
	 * @details 3Dモデルとレティクル等の描画を行います。
	 */
	void Draw() override;
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

	/** @brief ImGuiの描画処理（デバッグ用）。 */
	void DrawImGui();

	/**
	 * @brief 弾を撃つ処理。
	 * @details 発射間隔や弾生成、弾初期位置/方向の設定を行います。
	 */
	void Shoot();

	/**
	 * @brief 移動処理。
	 * @details 入力と地形（MapChip）に基づき位置を更新します。
	 */
	void Move();

	/**
	 * @brief 回転処理。
	 * @details 入力や照準に基づきプレイヤーの向きを更新します。
	 */
	void Rotate();

	/** @brief レティクル描画処理（2D）。 */
	void ReticleDraw();

	/**
	 * @brief ダッシュ/回避のリング演出を即時発火します。
	 */
	void TriggerDashRing();

	/**
	 * @brief 斜め視点でもレティクル通りに飛ばすための方向取得関数（地面へレイキャスト）。
	 * @return レティクルに対応する照準方向ベクトル（正規化される想定）。
	 */
	TuboEngine::Math::Vector3 GetAimDirectionFromReticle() const;


private:
	// --- 回避関連 ---
	/** @brief 回避開始処理（内部用）。 */
	void StartDodge();
	/** @brief 回避更新処理（内部用）。 */
	void UpdateDodge();
	/**
	 * @brief 回避可能か判定します（内部用）。
	 * @return 回避可能ならtrue。
	 */
	bool CanDodge() const;
	/**
	 * @brief 入力から回避方向を算出します（内部用）。
	 * @return 回避方向ベクトル。
	 */
	TuboEngine::Math::Vector3 GetDodgeInputDirection() const;

public:
	///-----------------------------------
	///				ゲッター
	///------------------------------------

	/** @brief プレイヤーの位置を取得します。 @return 位置。 */
	TuboEngine::Math::Vector3 GetPosition() const { return position; }
	/** @brief プレイヤーの回転を取得します。 @return 回転。 */
	TuboEngine::Math::Vector3 GetRotation() const { return rotation; }
	/** @brief プレイヤーのスケールを取得します。 @return スケール。 */
	TuboEngine::Math::Vector3 GetScale() const { return scale; }
	/** @brief プレイヤーの速度を取得します。 @return 速度。 */
	TuboEngine::Math::Vector3 GetVelocity() const { return velocity; }
	/** @brief プレイヤーのHPを取得します。 @return HP。 */
	int GetHP() const { return HP; }
	/** @brief 生存状態を取得します。 @return 生存中ならtrue。 */
	bool GetIsAlive() const { return isAlive; }
	/** @brief プレイヤーの弾リストを取得します。 @return 弾リスト参照。 */
	const std::vector<std::unique_ptr<PlayerBullet>>& GetBullets() const { return bullets; }
	/** @brief ダッシュ中か取得します。 @return ダッシュ中ならtrue。 */
	bool IsDashing() const { return isDashing_; } // 既存なら流用、無ければダミー
	/** @brief 被弾フラグを取得します。 @return 被弾していればtrue。 */
	bool GetIsHit() const { return isHit; } // 被弾フラグのゲッター

	///-----------------------------------
	///				セッター
	///-------------------------------------

	/** @brief プレイヤーの位置を設定します。 @param position 位置。 */
	void SetPosition(const TuboEngine::Math::Vector3& position) { this->position = position; }
	/** @brief プレイヤーの回転を設定します。 @param rotation 回転。 */
	void SetRotation(const TuboEngine::Math::Vector3& rotation) { this->rotation = rotation; }
	/** @brief プレイヤーのスケールを設定します。 @param scale スケール。 */
	void SetScale(const TuboEngine::Math::Vector3& scale) { this->scale = scale; }
	/** @brief プレイヤーの速度を設定します。 @param velocity 速度。 */
	void SetVelocity(const TuboEngine::Math::Vector3& velocity) { this->velocity = velocity; }
	/** @brief プレイヤーのHPを設定します。 @param HP HP。 */
	void SetHP(int HP) { this->HP = HP; }
	/** @brief 生存状態を設定します。 @param isAlive 生存中ならtrue。 */
	void SetIsDead(bool isAlive) { this->isAlive = isAlive; }
	/**
	 * @brief 使用するカメラを設定します。
	 * @param camera カメラ。
	 */
	void SetCamera(Camera* camera) { object3d->SetCamera(camera); camera_ = camera; }
	/** @brief ダッシュリングの前方オフセット量を設定します。 @param forward 前方オフセット量。 */
	void SetDashRingOffset(float forward) { dashRingOffsetForward_ = forward; }

	/**
	 * @brief モデルのアルファ（透明度）を設定します。
	 * @param alpha アルファ値。
	 */
	void SetModelAlpha(float alpha) {
		Vector4 color = object3d->GetModelColor();
		color.w = alpha;
		object3d->SetModelColor(color);
	}

	/**
	 * @brief 環境マップ（キューブマップ）を設定します。
	 * @param filePath キューブマップファイルパス。
	 */
	void CubeMapSet(const std::string& filePath) { object3d->SetCubeMapFilePath(filePath); }

	/**
	 * @brief マップチップフィールド参照を設定します。
	 * @param mapChipField マップチップフィールド。
	 */
	void SetMapChipField(MapChipField* mapChipField) { this->mapChipField = mapChipField; }

	/**
	 * @brief 移動可否（入力ロック）を設定します。
	 * @param flag ロックするならtrue。
	 */
	void SetMovementLocked(bool flag) { isMovementLocked = flag; }

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

	TuboEngine::Math::Vector3 dodgeDirection = {0.0f, 0.0f, 0.0f};        // 回避方向

	TuboEngine::Math::Vector3 position; // プレイヤーの位置
	TuboEngine::Math::Vector3 rotation; // プレイヤーの回転
	TuboEngine::Math::Vector3 scale;    // プレイヤーのスケール

	TuboEngine::Math::Vector3 velocity; // プレイヤーの速度
	int HP;           // プレイヤーのHP
	bool isHit;       // プレイヤーがヒットしたかどうか
	bool isAlive;      // プレイヤーの死亡状態

	//Reticle

	std::unique_ptr<Sprite> reticleSprite; // スプライト
	TuboEngine::Math::Vector2 reticlePosition = {0.0f, 0.0f}; // レティクルの位置（画面中央）
	TuboEngine::Math::Vector2 reticleSize = {50.0f, 50.0f};   // レティクルのサイズ

	bool isMovementLocked=false;

	// --- 追加: 移動軌跡用パーティクルエミッター ---
	IParticleEmitter* trailEmitter_ = nullptr; // ParticleManager生成管理。解放はマネージャに委譲
	TuboEngine::Math::Vector3 prevPositionTrail_{};              // 前フレーム位置
	IParticleEmitter* dashRingEmitter_ = nullptr;
	bool wasDashingPrev_ = false;
	bool isDashing_ = false; // 既存のダッシュ状態に置き換え可
	Camera* camera_ = nullptr; // 位置/方向参照用
	float dashRingOffsetForward_ = 0.0f; // カメラ前方方向へのオフセット量

	// 連続リング発生のためのタイマーと間隔
	float dodgeRingIntervalSec_ = 0.12f; // 回避中の連続発生間隔
	float dodgeRingEmitTimer_ = 0.0f;    // 次発生までの残り時間

	// Dash演出（ポストエフェクト）
	float dashPostEffectTimer_ = 0.0f;
	float dashPostEffectDuration_ = 0.25f;
	float dashRadialBlurPower_ = 0.06f; // 0.02がデフォルトなので少し強め

	// Damage演出（ポストエフェクト）
	// HP割合で常時かけるビネット（低HPほど強い）
	float lowHpVignetteMaxPower_ = 3.0f; // HP0付近での最大（デフォルト0.8→最大3.0）
	float lowHpVignetteStartRatio_ = 0.5f; // このHP割合以下から効き始める
	float lowHpVignetteSmoothing_ = 0.15f; // 追従のなめらかさ（0で即時）
	float lowHpVignetteCurrentPower_ = 0.8f; // 現在適用中（補間用）
};
