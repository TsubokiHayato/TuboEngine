#pragma once
#include "Camera.h"
#include "Player.h"

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

	Camera* GetCamera() const { return camera_; }

private:
	Player* target_ = nullptr;
	Vector3 offset_ = {0, 10, 0};
	Vector3 rotation_ = {DirectX::XM_PIDIV2, 0.0f, 0.0f}; // X軸90度回転（真上から）
	float followSpeed_ = 0.2f;
	Camera* camera_ = nullptr;
};
