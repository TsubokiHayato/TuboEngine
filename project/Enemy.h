#pragma once
#include "application/BaseCharacter.h"

class Enemy : public BaseCharacter {
	///---------------------------------------
	///				メンバ関数
	///-----------------------------------------
public:
	Enemy();
	~Enemy() override;

	// 初期化のオーバーライド
	void Initialize(Object3dCommon* object3dCommon) override;
	// 更新処理のオーバーライド
	void Update() override;
	// 描画処理のオーバーライド
	void Draw() override;
	// ImGuiの描画処理
	void DrawImGui();

	// 衝突時の処理のオーバーライド
	void OnCollision(Collider* other) override;

	// 当たり判定の中心座標を取得のオーバーライド
	Vector3 GetCenterPosition() const override;

	void Move(); // 移動処理

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

	// 座標、回転、スケール
	Vector3 GetPosition() const { return position; }
	void SetPosition(const Vector3& pos) { position = pos; }
	Vector3 GetRotation() const { return rotation; }
	void SetRotation(const Vector3& rot) { rotation = rot; }
	Vector3 GetScale() const { return scale; }
	void SetScale(const Vector3& scl) { scale = scl; }

	// 生存フラグ
	bool GetIsAlive() const { return isAlive; }
	void SetIsAlive(bool alive) { isAlive = alive; }

	//HP
	int GetHP() const { return HP; }
	void SetHP(int hp) { HP = hp; }

	///---------------------------------------
	///				メンバ変数
	///---------------------------------------
private:
	Vector3 position;                   // 敵の位置
	Vector3 rotation;                   //  // 敵の回転
	Vector3 scale = {1.0f, 1.0f, 1.0f}; // 初期スケール
	Vector3 velocity;                   // 敵の移動速度
	int HP = 100;                       // 敵のHP
	bool isAlive = true;                // 敵が生きているかどうかのフラグ
	bool isHit = false;                 // 衝突判定フラグ

	std::unique_ptr<Object3d> object3d; // 3Dオブジェクト
};
