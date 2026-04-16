#pragma once
#include "Camera.h"
#include "Vector3.h"
#include "Character/Player/Player.h"

#include <algorithm> // std::clamp
#include <cfloat>    // FLT_MAX
#include <memory>

class FollowTopDownCamera {
public:
	FollowTopDownCamera();
	~FollowTopDownCamera();

	void Initialize(Player* target, const TuboEngine::Math::Vector3& offset, float followSpeed = 0.2f);
	void Update();
	void DrawImGui();

	void SetRotation(const TuboEngine::Math::Vector3& rotation);
	void SetTarget(Player* target);
	void SetOffset(const TuboEngine::Math::Vector3& offset);
	void SetFollowSpeed(float speed);
	void SetLookAtOffset(const TuboEngine::Math::Vector3& offset);
	void SetZoom(float zoom);
	void SetBounds(const TuboEngine::Math::Vector3& min, const TuboEngine::Math::Vector3& max);
	void Shake(float intensity, float duration);
	void SetZoomLimits(float minZoom, float maxZoom) {
		zoomMin_ = minZoom;
		zoomMax_ = maxZoom;
	}
	void SetZoomSpeed(float speed) { zoomSpeed_ = speed; }

	// 開始時ズームアニメーション
	// durationSec: 秒（Updateは60FPS前提で内部変換）
	void StartIntroZoom(float startZoom, float endZoom, float durationSec);
	// curve: 1.0=標準、>1.0で強め（ゆっくり始まって最後に加速）、<1.0で弱め
	void SetIntroZoomCurve(float curve) { introZoomCurve_ = (curve < 0.01f) ? 0.01f : curve; }
	float GetIntroZoomCurve() const { return introZoomCurve_; }
	bool IsIntroZoomPlaying() const { return introZoomPlaying_; }

	TuboEngine::Camera* GetCamera() const { return camera_.get(); }
	TuboEngine::Math::Vector3 GetOffset() const { return offset_; }
	TuboEngine::Math::Vector3 GetRotation() const { return rotation_; }
	float GetZoom() const { return zoom_; }
	float GetZoomMin() const { return zoomMin_; }

	void SnapToTarget();

	// イントロズームアニメーションの進行状況
	float GetIntroZoomElapsedSec() const { return introZoomElapsedSec_; }
	float GetIntroZoomDurationSec() const { return introZoomDurationSec_; }
	float GetIntroZoomStart() const { return introZoomStart_; }
	float GetIntroZoomEnd() const { return introZoomEnd_; }
	float GetIntroZoomT() const {
		return (introZoomDurationSec_ > 0.0f) ? std::clamp(introZoomElapsedSec_ / introZoomDurationSec_, 0.0f, 1.0f) : 1.0f;
	}

private:
	Player* target_ = nullptr;
	TuboEngine::Math::Vector3 offset_ = {0, 10, 0};
	TuboEngine::Math::Vector3 lookAtOffset_ = {0, 0, 0};
	float followSpeed_ = 0.07f;
	float zoom_ = 1.0f;
	TuboEngine::Math::Vector3 rotation_ = {DirectX::XM_PIDIV2 * 1.5f, 0.0f, 0.0f};
	std::unique_ptr<TuboEngine::Camera> camera_;

	// カメラシェイク
	float shakeTime_ = 0.0f;
	float shakeDuration_ = 0.0f;
	float shakeIntensity_ = 0.0f;

	// 境界制限
	bool useBounds_ = false;
	TuboEngine::Math::Vector3 boundsMin_ = {-FLT_MAX, -FLT_MAX, -FLT_MAX};
	TuboEngine::Math::Vector3 boundsMax_ = {FLT_MAX, FLT_MAX, FLT_MAX};

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
	float introZoomCurve_ = 1.0f;

	// 障害物回避（雛形）
	void AvoidObstacles(TuboEngine::Math::Vector3& desiredPos);

	// シェイク用乱数
	float RandomFloat(float min, float max);

	// 内部: イージング
	static float EaseInOutCubic(float t);
	static float EaseWithCurve(float t, float curve);
};
