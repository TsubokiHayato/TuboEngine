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

	void SetRotation(const Vector3& rotation);
	void SetTarget(Player* target);
	void SetOffset(const Vector3& offset);
	void SetFollowSpeed(float speed);
	void SetLookAtOffset(const Vector3& offset);
	void SetZoom(float zoom);
	void SetBounds(const Vector3& min, const Vector3& max);
	void Shake(float intensity, float duration);
	void SetZoomLimits(float minZoom, float maxZoom) {
		zoomMin_ = minZoom;
		zoomMax_ = maxZoom;
	}
	void SetZoomSpeed(float speed) { zoomSpeed_ = speed; }

	// 開始時ズームアニメーション
	// durationSec: 秒（Updateは60FPS前提で内部変換）
	void StartIntroZoom(float startZoom, float endZoom, float durationSec);
	bool IsIntroZoomPlaying() const { return introZoomPlaying_; }

	Camera* GetCamera() const { return camera_; }
	Vector3 GetOffset() const { return offset_; }
	Vector3 GetRotation() const { return rotation_; }
	float GetZoom() const { return zoom_; }
	float GetZoomMin() const { return zoomMin_; }

	void SnapToTarget();

private:
	Player* target_ = nullptr;
	Vector3 offset_ = {0, 10, 0};
	Vector3 lookAtOffset_ = {0, 0, 0};
	float followSpeed_ = 0.07f;
	float zoom_ = 1.0f;
	Vector3 rotation_ = {DirectX::XM_PIDIV2 * 1.5f, 0.0f, 0.0f};
	Camera* camera_ = nullptr;

	// カメラシェイク
	float shakeTime_ = 0.0f;
	float shakeDuration_ = 0.0f;
	float shakeIntensity_ = 0.0f;

	// 境界制限
	bool useBounds_ = false;
	Vector3 boundsMin_ = {-FLT_MAX, -FLT_MAX, -FLT_MAX};
	Vector3 boundsMax_ = {FLT_MAX, FLT_MAX, FLT_MAX};

	// ズーム制限・速度
	float zoomMin_ = 0.5f;
	float zoomMax_ = 1.0f;
	float zoomSpeed_ = 0.05f; // ホイール1単位あたり?ズーム変化

	// 開始時ズームアニメーション（イージング）
	bool introZoomPlaying_ = false;
	float introZoomStart_ = 1.0f;
	float introZoomEnd_ = 1.0f;
	float introZoomDurationSec_ = 0.0f;
	float introZoomElapsedSec_ = 0.0f;

	// 障害物回避（雛形）
	void AvoidObstacles(Vector3& desiredPos);

	// シェイク用乱数
	float RandomFloat(float min, float max);

	// 内部: イージング
	static float EaseInOutCubic(float t);
};
