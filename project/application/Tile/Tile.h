#pragma once
#pragma once
#include "Object3d.h"
#include "Vector3.h"

class Tile {
public:
	Tile();
	~Tile();

	// 初期化（位置・スケール・モデルファイル名など）
	void Initialize( Vector3 position, const Vector3& scale = {1.0f, 1.0f, 1.0f}, const std::string& modelFileName = "tile/tile.gltf");

	void Update();
	// 描画
	void Draw();
	void DrawImGui(const char* windowName = "Tile");

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
	Vector3 position_ = {};
	Vector3 scale_ = {};
	Vector3 rotation_ = {};
	std::unique_ptr<Object3d> object3d_ = {};

	// まとめタイルのみ描画するためのフラグ（最初に生成されたTileだけ描画）
	bool isPrimary_ = false;
	static bool sPrimaryAssigned_;
};