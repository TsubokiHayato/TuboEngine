#include "FollowTopDownCamera.h"
#include "ImGuiManager.h"
#include "Input.h"
#include <DirectXMath.h>
#include <algorithm>
#include <cfloat>
#include <cmath>

namespace {
	constexpr float kAssumedDeltaTimeSec = 1.0f / 60.0f;
}

FollowTopDownCamera::FollowTopDownCamera() {}

FollowTopDownCamera::~FollowTopDownCamera() {
	delete camera_;
	camera_ = nullptr;
}

float FollowTopDownCamera::EaseInOutCubic(float t) {
	t = std::clamp(t, 0.0f, 1.0f);
	return (t < 0.5f) ? (4.0f * t * t * t) : (1.0f - std::pow(-2.0f * t + 2.0f, 3.0f) / 2.0f);
}

float FollowTopDownCamera::EaseWithCurve(float t, float curve) {
	// tの進行をカーブさせる（curve>1で序盤ゆっくり/終盤速い、curve<1でその逆）
	return std::pow(std::clamp(t, 0.0f, 1.0f), std::max(0.01f, curve));
}

void FollowTopDownCamera::StartIntroZoom(float startZoom, float endZoom, float durationSec) {
	introZoomPlaying_ = true;
	introZoomStart_ = startZoom;
	introZoomEnd_ = endZoom;
	introZoomDurationSec_ = std::max(0.001f, durationSec);
	introZoomElapsedSec_ = 0.0f;
	zoom_ = std::max(zoomMin_, std::min(zoomMax_, introZoomStart_));
}

void FollowTopDownCamera::Initialize(Player* target, const Vector3& offset, float followSpeed) {
	target_ = target;
	offset_ = offset;
	followSpeed_ = followSpeed;
	if (camera_) {
		delete camera_;
	}
	camera_ = new Camera();
	if (target_) {
		camera_->SetTranslate(target_->GetPosition() + offset_);
	} else {
		camera_->SetTranslate(offset_);
	}
	camera_->setRotation(rotation_);

	// 初期化時はイントロ状態をリセット
	introZoomPlaying_ = false;
	introZoomElapsedSec_ = 0.0f;
	introZoomDurationSec_ = 0.0f;
	introZoomStart_ = zoom_;
	introZoomEnd_ = zoom_;
	introZoomCurve_ = 5.0f;
}

void FollowTopDownCamera::SnapToTarget() {
	if (!target_ || !camera_) {
		return;
	}

	// target位置に対して、現在の回転/ズームを反映したオフセットで即配置
	Vector3 rotatedOffset = offset_;
	{
		using namespace DirectX;
		XMVECTOR off = XMVectorSet(offset_.x, offset_.y, offset_.z, 0.0f);
		XMVECTOR rotZ = XMQuaternionRotationAxis(XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f), rotation_.z);
		XMVECTOR rotX = XMQuaternionRotationAxis(XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f), rotation_.x);
		XMVECTOR rotY = XMQuaternionRotationAxis(XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f), rotation_.y);
		XMVECTOR rotQ = XMQuaternionMultiply(XMQuaternionMultiply(rotZ, rotX), rotY);
		XMVECTOR offR = XMVector3Rotate(off, rotQ);
		rotatedOffset.x = XMVectorGetX(offR);
		rotatedOffset.y = XMVectorGetY(offR);
		rotatedOffset.z = XMVectorGetZ(offR);
	}

	Vector3 desiredPos = target_->GetPosition() + rotatedOffset * zoom_;
	AvoidObstacles(desiredPos);

	if (useBounds_) {
		desiredPos.x = std::max(boundsMin_.x, std::min(boundsMax_.x, desiredPos.x));
		desiredPos.y = std::max(boundsMin_.y, std::min(boundsMax_.y, desiredPos.y));
		desiredPos.z = std::max(boundsMin_.z, std::min(boundsMax_.z, desiredPos.z));
	}

	camera_->SetTranslate(desiredPos);
	camera_->setRotation(rotation_);
	camera_->setScale({1.0f, 1.0f, 1.0f});
	camera_->Update();
}

void FollowTopDownCamera::Update() {
	if (!target_ || !camera_)
		return;

	// イントロズーム（イージング）
	if (introZoomPlaying_) {
		introZoomElapsedSec_ += kAssumedDeltaTimeSec;
		float t = introZoomElapsedSec_ / introZoomDurationSec_;
		float tc = EaseWithCurve(t, introZoomCurve_);
		float e = EaseInOutCubic(tc);
		zoom_ = introZoomStart_ + (introZoomEnd_ - introZoomStart_) * e;
		zoom_ = std::max(zoomMin_, std::min(zoomMax_, zoom_));
		if (t >= 1.0f) {
			introZoomPlaying_ = false;
			zoom_ = std::max(zoomMin_, std::min(zoomMax_, introZoomEnd_));
		}
	} else {
		// ホイールズーム更新（下限・上限を保持）
		int wheel = Input::GetInstance()->GetWheel();
		if (wheel != 0) {
			float delta = (wheel > 0 ? -1.0f : 1.0f) * zoomSpeed_;
			zoom_ = std::max(zoomMin_, std::min(zoomMax_, zoom_ + delta));
		}
	}

	// プレイヤーの位置を取得
	Vector3 targetPos = target_->GetPosition();

	// 注視点オフセット
	Vector3 lookAt = targetPos + lookAtOffset_;

	// 回転に応じてオフセットを回転させる（斜め視点対応）
	// rotation_はラジアン想定。順序: Z->X->Y（右手座標系）
	Vector3 rotatedOffset = offset_;
	{
		using namespace DirectX;
		XMVECTOR off = XMVectorSet(offset_.x, offset_.y, offset_.z, 0.0f);
		XMVECTOR rotZ = XMQuaternionRotationAxis(XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f), rotation_.z);
		XMVECTOR rotX = XMQuaternionRotationAxis(XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f), rotation_.x);
		XMVECTOR rotY = XMQuaternionRotationAxis(XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f), rotation_.y);
		XMVECTOR rotQ = XMQuaternionMultiply(XMQuaternionMultiply(rotZ, rotX), rotY);
		XMVECTOR offR = XMVector3Rotate(off, rotQ);
		rotatedOffset.x = XMVectorGetX(offR);
		rotatedOffset.y = XMVectorGetY(offR);
		rotatedOffset.z = XMVectorGetZ(offR);
	}

	// 目標カメラ位置（ズーム対応）
	Vector3 desiredPos = targetPos + rotatedOffset * zoom_;

	// 障害物回避（雛形）
	AvoidObstacles(desiredPos);

	// 境界制限
	if (useBounds_) {
		desiredPos.x = std::max(boundsMin_.x, std::min(boundsMax_.x, desiredPos.x));
		desiredPos.y = std::max(boundsMin_.y, std::min(boundsMax_.y, desiredPos.y));
		desiredPos.z = std::max(boundsMin_.z, std::min(boundsMax_.z, desiredPos.z));
	}

	// 線形補間で滑らかに追従
	Vector3 currentPos = camera_->GetTranslate();
	Vector3 newPos = currentPos + (desiredPos - currentPos) * followSpeed_;

	// カメラシェイク
	if (shakeTime_ > 0.0f) {
		newPos.x += RandomFloat(-shakeIntensity_, shakeIntensity_);
		newPos.y += RandomFloat(-shakeIntensity_, shakeIntensity_);
		newPos.z += RandomFloat(-shakeIntensity_, shakeIntensity_);
		shakeTime_ -= kAssumedDeltaTimeSec; // 60FPS前提
		if (shakeTime_ < 0.0f)
			shakeTime_ = 0.0f;
	}

	camera_->SetTranslate(newPos);
	camera_->setRotation(rotation_);
	camera_->setScale({1.0f, 1.0f, 1.0f});
	camera_->Update();
}

void FollowTopDownCamera::DrawImGui() {

#ifdef USE_IMGUI
	ImGui::Begin("Follow Top Down Camera");
	ImGui::DragFloat3("Offset", &offset_.x, 0.1f);
	ImGui::DragFloat3("LookAtOffset", &lookAtOffset_.x, 0.1f);
	ImGui::DragFloat3("Rotation", &rotation_.x, 0.01f);
	ImGui::DragFloat("Follow Speed", &followSpeed_, 0.01f, 0.0f, 1.0f);
	ImGui::DragFloat("Zoom", &zoom_, 0.01f, zoomMin_, zoomMax_);
	ImGui::DragFloat("Zoom Speed", &zoomSpeed_, 0.001f, 0.0f, 0.2f);
	ImGui::DragFloat("Zoom Min", &zoomMin_, 0.01f, 0.1f, 10.0f);
	ImGui::DragFloat("Zoom Max", &zoomMax_, 0.01f, 0.1f, 10.0f);
	ImGui::Checkbox("Use Bounds", &useBounds_);
	ImGui::DragFloat3("Bounds Min", &boundsMin_.x, 0.1f);
	ImGui::DragFloat3("Bounds Max", &boundsMax_.x, 0.1f);
	if (ImGui::Button("Shake")) {
		Shake(0.5f, 0.3f);
	}
	ImGui::Text("Camera Position: (%.2f, %.2f, %.2f)", camera_->GetTranslate().x, camera_->GetTranslate().y, camera_->GetTranslate().z);
	ImGui::Text("Camera Rotation: (%.2f, %.2f, %.2f)", rotation_.x, rotation_.y, rotation_.z);

	ImGui::SeparatorText("Intro Zoom");
	ImGui::Text("Playing: %s", introZoomPlaying_ ? "true" : "false");
	ImGui::Text("Start: %.3f  End: %.3f", introZoomStart_, introZoomEnd_);
	ImGui::Text("Elapsed: %.3f / %.3f sec", introZoomElapsedSec_, introZoomDurationSec_);
	const float t = GetIntroZoomT();
	ImGui::ProgressBar(t, ImVec2(-1.0f, 0.0f), "t");
	ImGui::DragFloat("Curve", &introZoomCurve_, 0.01f, 0.1f, 5.0f);
	const float tc = EaseWithCurve(t, introZoomCurve_);
	const float e = EaseInOutCubic(tc);
	ImGui::Text("t(curved)=%.3f  ease=%.3f", tc, e);
	if (ImGui::Button("Restart Intro (Debug)")) {
		StartIntroZoom(introZoomStart_, introZoomEnd_, std::max(0.001f, introZoomDurationSec_));
	}

	ImGui::End();
#endif // USE_IMGUI
}

void FollowTopDownCamera::SetRotation(const Vector3& rotation) { rotation_ = rotation; }
void FollowTopDownCamera::SetTarget(Player* target) { target_ = target; }
void FollowTopDownCamera::SetOffset(const Vector3& offset) { offset_ = offset; }
void FollowTopDownCamera::SetFollowSpeed(float speed) { followSpeed_ = speed; }
void FollowTopDownCamera::SetLookAtOffset(const Vector3& offset) { lookAtOffset_ = offset; }
void FollowTopDownCamera::SetZoom(float zoom) { zoom_ = std::max(zoomMin_, std::min(zoomMax_, zoom)); }
void FollowTopDownCamera::SetBounds(const Vector3& min, const Vector3& max) {
	boundsMin_ = min;
	boundsMax_ = max;
	useBounds_ = true;
}
void FollowTopDownCamera::Shake(float intensity, float duration) {
	shakeIntensity_ = intensity;
	shakeDuration_ = duration;
	shakeTime_ = duration;
}

// 障害物回避の雛形（実装はゲームごとに要カスタマイズ）
void FollowTopDownCamera::AvoidObstacles(Vector3& desiredPos) {
	// ここにレイキャスト等で障害物を検出し、desiredPosを調整する処理を実装
	// 今は何もしない
}

// シェイク用乱数
float FollowTopDownCamera::RandomFloat(float min, float max) { return min + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX) / (max - min)); }
