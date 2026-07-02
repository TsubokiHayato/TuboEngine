#pragma once

#include <memory>
#include <string>

#include "Input.h"
#include "Sprite.h"

/// <summary>
/// 操作方法（キー・マウス）のガイドを画面に表示するUI。入力状態に応じてアイコンの表示を切り替える。
/// </summary>
class GuideUI {
public:
	/// <summary>
	/// 初期化処理。
	/// </summary>
	void Initialize();
	/// <summary>
	/// 更新処理。
	/// </summary>
	void Update();
	/// <summary>
	/// 描画処理。
	/// </summary>
	void Draw();

	/// <summary>
	/// 有効フラグの取得・設定。
	/// </summary>
	void SetActive(bool active) { isActive_ = active; }
	bool IsActive() const { return isActive_; }

	/// <summary>
	/// 座標の取得・設定。
	/// </summary>
	void SetPosition(const TuboEngine::Math::Vector2& leftBottom) { basePos_ = leftBottom; }
	const TuboEngine::Math::Vector2& GetPosition() const { return basePos_; }

private:
	/// <summary>
	/// キーの押下状態に応じてスプライトのテクスチャを切り替える。
	/// </summary>
	void UpdateKeySpriteState(TuboEngine::Sprite* sprite, const std::string& normalTex, const std::string& outlineTex, bool pressed) const;
	/// <summary>
	/// マウス移動量に応じたアイコンテクスチャを選択する。
	/// </summary>
	const std::string& SelectMouseIconTexture(const TuboEngine::Input::MouseMove& move) const;

private:
	bool isActive_ = true;
	TuboEngine::Math::Vector2 basePos_{20.0f, 720.0f - 20.0f}; // will be converted to left-bottom in Initialize

	std::unique_ptr<TuboEngine::Sprite> labelMove_;
	std::unique_ptr<TuboEngine::Sprite> labelDash_;
	std::unique_ptr<TuboEngine::Sprite> labelAim_;
	std::unique_ptr<TuboEngine::Sprite> labelShoot_;

	std::unique_ptr<TuboEngine::Sprite> keyW_;
	std::unique_ptr<TuboEngine::Sprite> keyA_;
	std::unique_ptr<TuboEngine::Sprite> keyS_;
	std::unique_ptr<TuboEngine::Sprite> keyD_;
	std::unique_ptr<TuboEngine::Sprite> keySpace_;
	std::unique_ptr<TuboEngine::Sprite> mouseIcon_;
	std::unique_ptr<TuboEngine::Sprite> mouseShootIcon_;

	// texture paths
	std::string texW_;
	std::string texWOutline_;
	std::string texA_;
	std::string texAOutline_;
	std::string texS_;
	std::string texSOutline_;
	std::string texD_;
	std::string texDOutline_;
	std::string texSpace_;
	std::string texSpaceOutline_;
	std::string texMouseSmall_;
	std::string texMouseMove_;
	std::string texMouse_;
	std::string texMouseRight_;
};
