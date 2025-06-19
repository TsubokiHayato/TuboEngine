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

	void Move() override;
	void TakeDamage(int damage) override;

	// 必要に応じてEnemy固有のメンバ変数・関数を追加


	///------------------------------------------------------
	///				ゲッター&セッター
	///------------------------------------------------------
public:
	void SetCamera(Camera* camera) {
		if (object3d) {
			object3d->SetCamera(camera);
		}
	}

	Vector3 GetPosition() const { return position; }
	void SetPosition(const Vector3& pos) { position = pos; }
	Vector3 GetRotation() const { return rotation; }
	void SetRotation(const Vector3& rot) { rotation = rot; }
	Vector3 GetScale() const { return scale; }
	void SetScale(const Vector3& scl) { scale = scl; }

	bool GetIsAlive() const { return isAlive; }
	void SetIsAlive(bool alive) { isAlive = alive; }

	///---------------------------------------
	///				メンバ変数
	///---------------------------------------
private:
	Vector3 position;
	Vector3 rotation;
	Vector3 scale = {1.0f, 1.0f, 1.0f}; // 初期スケール
	Vector3 velocity;
	int HP = 100;
	bool isAlive = true;

	std::unique_ptr<Object3d> object3d;
};
