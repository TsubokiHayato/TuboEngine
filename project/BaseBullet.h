#pragma once
#include "Object3d.h"
#include "Object3dCommon.h"
#include "Vector3.h"
#include "Collider.h"

class BaseBullet :public Collider {
public:
	virtual ~BaseBullet() = default;

	virtual void Initialize(Object3dCommon* object3dCommon, const Vector3& startPos, const Vector3& startVel) = 0;
	virtual void Update() = 0;
	virtual void Draw() = 0;

	/// <summary>
	///　衝突判定
	/// </summary>
	/// <param name="other"></param>
	virtual void OnCollision(Collider* other) override;

	/// <summary>
	/// 当たり判定の中心座標を取得
	virtual Vector3 GetCenterPosition() const override;

	bool IsAlive() const { return isAlive; }

protected:
	Vector3 position;
	Vector3 velocity;
	Vector3 rotation;
	Vector3 scale;
	bool isAlive = false;
	std::unique_ptr<Object3d> object3d;
};
