#pragma once
#include "Object3d.h"
#include "Object3dCommon.h"
#include "Vector3.h"
#include "Collider/Collider.h"

class BaseBullet :public Application::Collider {
public:
	virtual ~BaseBullet() = default;

	virtual void Initialize(const TuboEngine::Math::Vector3& startPos) = 0;
	virtual void Update() = 0;
	virtual void Draw() = 0;


	/// <summary>
	///　衝突判定
	/// </summary>
	/// <param name="other"></param>
	virtual void OnCollision(Collider* other) override;

	/// <summary>
	/// 当たり判定の中心座標を取得
	virtual TuboEngine::Math::Vector3 GetCenterPosition() const override;

	bool IsAlive() const { return isAlive; }

protected:
	TuboEngine::Math::Vector3 position;
	TuboEngine::Math::Vector3 velocity;
	TuboEngine::Math::Vector3 rotation;
	TuboEngine::Math::Vector3 scale;
	bool isAlive = false;
	std::unique_ptr<Object3d> object3d;
};
