#pragma once
#include "Camera.h"
#include "Character/Player/Player.h"

class FollowTopDownCamera {
public:
	FollowTopDownCamera();
	~FollowTopDownCamera();

	void Initialize(Player* target, const Vector3& offset, float followSpeed = 0.2f);
	void Update();
	void DrawImGui();

	void SetTarget(Player* target);
	void SetOffset(const Vector3& offset);
	void SetFollowSpeed(float speed);
	void SetLookAtOffset(const Vector3& offset);
	void SetZoom(float zoom);
	void SetBounds(const Vector3& min, const Vector3& max);
	void Shake(float intensity, float duration);

	Camera* GetCamera() const { return camera_; }

private:
	Player* target_ = nullptr;
	Vector3 offset_ = {0, 10, 0};
	Vector3 lookAtOffset_ = {0, 0, 0};
	float followSpeed_ = 0.2f;
	float zoom_ = 1.0f;
	Vector3 rotation_ = {DirectX::XM_PIDIV2 * 2.0f,0.0f, 0.0f};
	Camera* camera_ = nullptr;

	// カメラシェイク
	float shakeTime_ = 0.0f;
	float shakeDuration_ = 0.0f;
	float shakeIntensity_ = 0.0f;

	// 境界制限
	bool useBounds_ = false;
	Vector3 boundsMin_ = {-FLT_MAX, -FLT_MAX, -FLT_MAX};
	Vector3 boundsMax_ = {FLT_MAX, FLT_MAX, FLT_MAX};

	// 障害物回避（雛形）
	void AvoidObstacles(Vector3& desiredPos);

	// シェイク用乱数
	float RandomFloat(float min, float max);
};
