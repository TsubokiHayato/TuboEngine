#pragma once
#include "Vector3.h"
#include "Object3d.h"
#include "Object3dCommon.h"
#include "BaseBullet.h"
 
class PlayerBullet : public BaseBullet {
public:
	
	///--------------------------------------------------
	///				メンバ関数
	///

	void Initialize(Object3dCommon* object3dCommon, const Vector3& startPos, const Vector3& startVel) override;
	void Update() override;
	void Draw() override;

	/// <summary>
	/// 　衝突判定
	/// </summary>
	/// <param name="other"></param>
	virtual void OnCollision(Collider* other) override;

	/// <summary>
	/// 当たり判定の中心座標を取得
	virtual Vector3 GetCenterPosition() const override;
	///--------------------------------------------------
	///				ゲッター&セッター
	/// 

	// 生存判定
	bool GetIsAlive() const { return isAlive; }

	// 位置取得
	const Vector3& GetPosition() const { return position; }
	// 位置設定
	void SetPosition(const Vector3& position) { this->position = position; }
	// 回転取得
	const Vector3& GetRotation() const { return rotation; }
	// 回転設定
	void SetRotation(const Vector3& rotation) { this->rotation = rotation; }
	// スケール取得
	const Vector3& GetScale() const { return scale; }
	// スケール設定
	void SetScale(const Vector3& scale) { this->scale = scale; }
	// 速度取得
	const Vector3& GetVelocity() const { return velocity; }
	// 速度設定
	void SetVelocity(const Vector3& velocity) { this->velocity = velocity; }
	// カメラ設定
	void SetCamera(Camera* camera) { object3d->SetCamera(camera); }


private:
	///--------------------------------------------------
	///				メンバ変数
	///
	// 弾のオブジェクト
	std::unique_ptr<Object3d> object3d;
	

	// 弾の位置
	Vector3 position;
	// 弾の回転
	Vector3 rotation;
	// 弾のスケール
	Vector3 scale;


	// 弾の速度
	Vector3 velocity;
	// 生存フラグ
	bool isAlive = true;
	//
	bool isHit = false;
};
