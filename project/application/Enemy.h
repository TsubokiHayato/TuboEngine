#pragma once
#include "BaseCharacter.h"

class Enemy : public BaseCharacter {
	///---------------------------------------
	///				メンバ関数
	///-----------------------------------------
public:
	Enemy();
	~Enemy() override;

	void Initialize(Object3dCommon* object3dCommon) override;
	void Update() override;
	void Draw() override;
	void DrawImGui();

	void Move();

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
	void SetCamera(Camera* camera) {
		if (object3d) {
			object3d->SetCamera(camera);
		}
	}

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

	///---------------------------------------
	///				メンバ変数
	///---------------------------------------
private:
	Vector3 position;                   // 初期位置
	Vector3 rotation;                   // 初期回転
	Vector3 scale = {1.0f, 1.0f, 1.0f}; // 初期スケール
	Vector3 velocity;                   // プレイヤーの速度
	int HP = 100;                       // 敵のHP
	bool isAlive = true;                // 敵が生きているかどうかのフラグ
	bool isHit = false;                 // 衝突判定フラグ

	std::unique_ptr<Object3d> object3d; // 3Dオブジェクト
};
