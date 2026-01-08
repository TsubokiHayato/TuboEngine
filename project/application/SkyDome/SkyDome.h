#pragma once
#include "Object3d.h"
#include "Vector3.h"

class SkyDome {
public:
	SkyDome();
	~SkyDome();

	// 初期化（位置・スケール・モデルファイル名など）
	void Initialize(const Vector3& position = {0.0f,0.0f,0.0f}, const Vector3& scale = {1.0f, 1.0f, 1.0f}, const std::string& modelFileName = "skyBox/skyBox.obj");

	void Update();
	// 描画
	void Draw();
	void DrawImGui(const char* windowName = "SkyBox");

public:
	// 位置取得・設定
	Vector3 GetPosition() const { return position_; }
	void SetPosition(const Vector3& pos) {
		position_ = pos;
		object3d_->SetPosition(pos);
	}

	// スケール取得・設定
	Vector3 GetScale() const { return scale_; }
	void SetScale(const Vector3& scale) {
		scale_ = scale;
		object3d_->SetScale(scale);
	}

	// 回転取得・設定
	Vector3 GetRotation() const { return rotation_; }
	void SetRotation(const Vector3& rot) {
		rotation_ = rot;
		object3d_->SetRotation(rot);
	}

	// カメラ設定
	void SetCamera(Camera* camera) { object3d_->SetCamera(camera); }

private:
	Vector3 position_;
	Vector3 scale_;
	Vector3 rotation_;
	std::unique_ptr<Object3d> object3d_;
};