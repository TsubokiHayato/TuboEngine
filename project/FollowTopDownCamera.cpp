#include "FollowTopDownCamera.h"
#include <DirectXMath.h>
#include"ImGuiManager.h"

FollowTopDownCamera::FollowTopDownCamera() {}

FollowTopDownCamera::~FollowTopDownCamera() {
	delete camera_;
	camera_ = nullptr;
}

void FollowTopDownCamera::Initialize(Player* target, const Vector3& offset, float followSpeed) {
	target_ = target;
	offset_ = offset;
	followSpeed_ = followSpeed;
	// カメラの初期化
	if (camera_) {
		delete camera_;
	}
	camera_ = new Camera();
	if (target_) {
		camera_->SetTranslate(target_->GetPosition() + offset_);
	} else {
		camera_->SetTranslate(offset_);
	}
	camera_->setRotation({DirectX::XM_PIDIV2, 0.0f, 0.0f}); // X軸90度回転（真上から）
}

void FollowTopDownCamera::Update() {
	if (!target_ || !camera_)
		return;

	// プレイヤーの位置を取得
	Vector3 targetPos = target_->GetPosition();

	// 目標カメラ位置（プレイヤーの上空から見下ろす）
	Vector3 desiredPos = targetPos + offset_;

	// 線形補間で滑らかに追従
	Vector3 currentPos = camera_->GetTranslate();
	Vector3 newPos = currentPos + (desiredPos - currentPos) * followSpeed_;

	camera_->SetTranslate(newPos);
	//camera_->setRotation({0.0f, 0.0f, 0.0f}); // X軸90度回転（真上から）
	camera_->setRotation(rotation_);
	camera_->setScale({1.0f, 1.0f, 1.0f}); // スケールを1に設定
	camera_->Update();
}

void FollowTopDownCamera::DrawImGui() {
	// ImGuiでカメラのパラメータを表示・調整する場合はここに記述
	ImGui::Begin("Follow Top Down Camera");
	ImGui::DragFloat3("Offset", &offset_.x, 0.1f);
	ImGui::DragFloat3("Rotation", &rotation_.x, 0.01f);
	ImGui::DragFloat("Follow Speed", &followSpeed_, 0.01f, 0.0f, 1.0f);
	ImGui::Text("Camera Position: (%.2f, %.2f, %.2f)", camera_->GetTranslate().x, camera_->GetTranslate().y, camera_->GetTranslate().z);
	ImGui::Text("Camera Rotation: (%.2f, %.2f, %.2f)", rotation_.x, rotation_.y, rotation_.z);
	ImGui::End();
}

void FollowTopDownCamera::SetTarget(Player* target) { target_ = target; }
void FollowTopDownCamera::SetOffset(const Vector3& offset) { offset_ = offset; }
void FollowTopDownCamera::SetFollowSpeed(float speed) { followSpeed_ = speed; }
