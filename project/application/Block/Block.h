#pragma once

#include "Object3d.h"
#include "Vector3.h"

#include <memory>
#include <string>

/// <summary>
/// ステージ上に配置するシンプルな 3D ブロック。
///
/// 位置/回転/スケールの状態を保持し、内部の `Object3d` に反映して描画する。
/// </summary>
class Block {
public:
	Block();
	~Block();

	/// <summary>
	/// 初期化。
	/// </summary>
	/// <param name="position">初期位置</param>
	/// <param name="scale">初期スケール</param>
	/// <param name="modelFileName">使用するモデルファイル</param>
	void Initialize(const TuboEngine::Math::Vector3& position, const TuboEngine::Math::Vector3& scale = {1.0f, 1.0f, 1.0f},
		const std::string& modelFileName = "block/block.obj");

	/// <summary>
	/// 更新（内部 `Object3d` の更新）。
	/// </summary>
	void Update();

	/// <summary>
	/// 描画。
	/// </summary>
	void Draw();

	/// <summary>
	/// デバッグUI。
	/// </summary>
	void DrawImGui(const char* windowName = "Block");

	/// <summary>
	/// 位置取得。
	/// </summary>
	TuboEngine::Math::Vector3 GetPosition() const { return position_; }

	/// <summary>
	/// 位置設定（内部 `Object3d` にも反映）。
	/// </summary>
	void SetPosition(const TuboEngine::Math::Vector3& pos);

	/// <summary>
	/// スケール取得。
	/// </summary>
	TuboEngine::Math::Vector3 GetScale() const { return scale_; }

	/// <summary>
	/// スケール設定（内部 `Object3d` にも反映）。
	/// </summary>
	void SetScale(const TuboEngine::Math::Vector3& scale);

	/// <summary>
	/// 回転取得。
	/// </summary>
	TuboEngine::Math::Vector3 GetRotation() const { return rotation_; }

	/// <summary>
	/// 回転設定（内部 `Object3d` にも反映）。
	/// </summary>
	void SetRotation(const TuboEngine::Math::Vector3& rot);

	/// <summary>
	/// 使用カメラを設定。
	/// </summary>
	void SetCamera(Camera* camera);

private:
	TuboEngine::Math::Vector3 position_{};
	TuboEngine::Math::Vector3 scale_{1.0f, 1.0f, 1.0f};
	TuboEngine::Math::Vector3 rotation_{};
	std::unique_ptr<TuboEngine::Object3d> object3d_;
};