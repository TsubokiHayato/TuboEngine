#pragma once
#include "Object3d.h"
#include "Vector3.h"

/// <summary>
/// ステージの床タイル1枚を表す3Dオブジェクト。
/// </summary>
class Tile {
public:
	/// <summary>
	/// コンストラクタ。
	/// </summary>
	Tile();
	/// <summary>
	/// デストラクタ。
	/// </summary>
	~Tile();

	// 初期化（位置・スケール・モデルファイル名など）
	void Initialize(TuboEngine::Math::Vector3 position,
		const TuboEngine::Math::Vector3& scale = {1.0f, 1.0f, 1.0f},
		const std::string& modelFileName = "tile/tile.gltf");

	/// <summary>
	/// 更新処理。
	/// </summary>
	void Update();
	// 描画
	void Draw();
	void DrawImGui(const char* windowName = "Tile");

public:
	// 位置取得・設定
	TuboEngine::Math::Vector3 GetPosition() const { return position_; }
	void SetPosition(const TuboEngine::Math::Vector3& pos) {
		position_ = pos;
		object3d_->SetPosition(pos);
	}

	// スケール取得・設定
	TuboEngine::Math::Vector3 GetScale() const { return scale_; }
	void SetScale(const TuboEngine::Math::Vector3& scale) {
		scale_ = scale;
		object3d_->SetScale(scale);
	}

	// 回転取得・設定
	TuboEngine::Math::Vector3 GetRotation() const { return rotation_; }
	void SetRotation(const TuboEngine::Math::Vector3& rot) {
		rotation_ = rot;
		object3d_->SetRotation(rot);
	}

	// カメラ設定
	void SetCamera(TuboEngine::Camera* camera) { object3d_->SetCamera(camera); }

	/// <summary>
	/// Object3d を取得する。
	/// </summary>
	TuboEngine::Object3d* GetObject3d() const { return object3d_.get(); }

private:
	TuboEngine::Math::Vector3 position_ = {};
	TuboEngine::Math::Vector3 scale_ = {};
	TuboEngine::Math::Vector3 rotation_ = {};
	std::unique_ptr<TuboEngine::Object3d> object3d_ = {};

	// まとめタイルのみ描画するためのフラグ（最初に生成されたTileだけ描画）
	bool isPrimary_ = false;
	static bool sPrimaryAssigned_;
};