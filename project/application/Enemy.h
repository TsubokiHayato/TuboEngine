#pragma once
#include "BaseCharacter.h"
#include "EnemyNormalBullet.h"
#include "Particle.h"
#include "ParticleEmitter.h"

///---------------------------------------------------
///				前方宣言
///---------------------------------------------------
class Player;

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
	bool GetIsAlive() const { return isAlive; }
	void SetIsAlive(bool alive) { isAlive = alive; }

	void SetPlayer(Player* player) { player_ = player; }

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
		Idle,   // 待機
		Move,   // 移動
		Shoot   // 射撃
	};

private:
	///-----Enemy-----///
	Vector3 position;                   // 初期位置
	Vector3 rotation;                   // 初期回転
	Vector3 scale = {1.0f, 1.0f, 1.0f}; // 初期スケール
	Vector3 velocity;                   // プレイヤーの速度
	int HP = 100;                       // 敵のHP
	bool isAlive = true;                // 敵が生きているかどうかのフラグ
	bool isHit = false;                 // 衝突判定フラグ
	bool wasHit = false;                // 前フレームのisHit
	float turnSpeed_ = 0.1f;            // プレイヤー方向を向く回転補間率（0.0f〜1.0f）
	std::unique_ptr<Object3d> object3d; // 3Dオブジェクト
	State state_ = State::Move;         // 行動状態（移動/射撃）
	float shootDistance_ = 7.0f;        // プレイヤーに近づく距離の閾値（例: 7.0f）
	float moveSpeed_ = 0.08f;           // 移動速度
	float moveStartDistance_ = 15.0f;   // 移動開始距離（これより遠いとIdle）

	///-----Player-----///
	Player* player_ = nullptr; // プレイヤーへのポインタ

	///-----Bullet-----///
	std::unique_ptr<EnemyNormalBullet> bullet; // 敵の弾

	///-----Particle-----///

	std::unique_ptr<Particle> particle;
	std::unique_ptr<ParticleEmitter> particleEmitter_;

	Transform particleTranslate;
	Vector3 particleVelocity = {0.0f, 0.0f, 0.0f};
	Vector4 particleColor = {1.0f, 1.0f, 1.0f, 1.0f};
	float particleLifeTime = 1.0f;
	float particleCurrentTime = 0.0f;
};
