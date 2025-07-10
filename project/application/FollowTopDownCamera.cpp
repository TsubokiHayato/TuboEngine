#include "FollowTopDownCamera.h"
#include "ImGuiManager.h"
#include <DirectXMath.h>
#include <algorithm>
#include <cfloat>

FollowTopDownCamera::FollowTopDownCamera() {}

FollowTopDownCamera::~FollowTopDownCamera() {
	delete camera_;
	camera_ = nullptr;
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
}

void FollowTopDownCamera::Update() {
	if (!target_ || !camera_)
		return;

	// プレイヤーの位置を取得
	Vector3 targetPos = target_->GetPosition();

	// 注視点オフセット
	Vector3 lookAt = targetPos + lookAtOffset_;

	// 目標カメラ位置（ズーム対応）
	Vector3 desiredPos = targetPos + offset_ * zoom_;

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
		shakeTime_ -= 1.0f / 60.0f; // 60FPS前提
		if (shakeTime_ < 0.0f)
			shakeTime_ = 0.0f;
	}

	camera_->SetTranslate(newPos);
	camera_->setRotation(rotation_);
	camera_->setScale({1.0f, 1.0f, 1.0f});
	camera_->Update();
}

void FollowTopDownCamera::DrawImGui() {
	ImGui::Begin("Follow Top Down Camera");
	ImGui::DragFloat3("Offset", &offset_.x, 0.1f);
	ImGui::DragFloat3("LookAtOffset", &lookAtOffset_.x, 0.1f);
	ImGui::DragFloat3("Rotation", &rotation_.x, 0.01f);
	ImGui::DragFloat("Follow Speed", &followSpeed_, 0.01f, 0.0f, 1.0f);
	ImGui::DragFloat("Zoom", &zoom_, 0.01f, 0.1f, 10.0f);
	ImGui::Checkbox("Use Bounds", &useBounds_);
	ImGui::DragFloat3("Bounds Min", &boundsMin_.x, 0.1f);
	ImGui::DragFloat3("Bounds Max", &boundsMax_.x, 0.1f);
	if (ImGui::Button("Shake")) {
		Shake(0.5f, 0.3f);
	}
	ImGui::Text("Camera Position: (%.2f, %.2f, %.2f)", camera_->GetTranslate().x, camera_->GetTranslate().y, camera_->GetTranslate().z);
	ImGui::Text("Camera Rotation: (%.2f, %.2f, %.2f)", rotation_.x, rotation_.y, rotation_.z);
	ImGui::End();
}

void FollowTopDownCamera::SetTarget(Player* target) { target_ = target; }
void FollowTopDownCamera::SetOffset(const Vector3& offset) { offset_ = offset; }
void FollowTopDownCamera::SetFollowSpeed(float speed) { followSpeed_ = speed; }
void FollowTopDownCamera::SetLookAtOffset(const Vector3& offset) { lookAtOffset_ = offset; }
void FollowTopDownCamera::SetZoom(float zoom) { zoom_ = zoom; }
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
