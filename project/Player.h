#pragma once
#include "BaseCharacter.h"
#include "PlayerBullet.h"

class Player : public BaseCharacter {
public:
	///--------------------------------------------------
	///				メンバ関数
	///--------------------------------------------------

	Player();
	~Player() override;

	// 初期化のオーバーライド
	void Initialize(Object3dCommon* object3dCommon) override;
	// 更新処理のオーバーライド
	void Update() override;

	// 弾を撃つ処理
	void Shoot();

	// 描画処理のオーバーライド
	void Draw() override;
	// 終了処理のオーバーライド
	void Finalize() override;

	// 移動処理のオーバーライド
	void Move() override;
	// ダメージ処理のオーバーライド
	void TakeDamage(int damage) override;

	void DrawImgui();

public:
	///-----------------------------------
	///				ゲッター
	///------------------------------------

	// プレイヤーの位置
	Vector3 GetPosition() const { return position; }
	// プレイヤーの速度
	Vector3 GetVelocity() const { return velocity; }
	// プレイヤーのHP
	int GetHP() const { return HP; }
	// プレイヤーの死亡状態
	bool IsDead() const { return isDead; }

	///-----------------------------------
	///				セッター
	///-------------------------------------

	// プレイヤーの位置
	void SetPosition(const Vector3& position) { this->position = position; }
	// プレイヤーの速度
	void SetVelocity(const Vector3& velocity) { this->velocity = velocity; }
	// プレイヤーのHP
	void SetHP(int HP) { this->HP = HP; }
	// プレイヤーの死亡状態
	void SetIsDead(bool isDead) { this->isDead = isDead; }
	// Camera
	void SetCamera(Camera* camera) { object3d->SetCamera(camera); }

private:
	///--------------------------------------------------
	///				引き渡し用変数
	///--------------------------------------------------
	Object3dCommon* object3dCommon_ = nullptr; // 3Dオブジェクト共通部分
private:
	///--------------------------------------------------
	///				メンバ変数
	///

	std::unique_ptr<Object3d> object3d;                 // 3Dオブジェクト
	std::vector<std::unique_ptr<PlayerBullet>> bullets; // プレイヤーの弾のリスト

	Vector3 position; // プレイヤーの位置
	Vector3 rotation; // プレイヤーの回転
	Vector3 scale;    // プレイヤーのスケール

	Vector3 velocity; // プレイヤーの速度
	int HP;           // プレイヤーのHP
	bool isDead;      // プレイヤーの死亡状態
};
