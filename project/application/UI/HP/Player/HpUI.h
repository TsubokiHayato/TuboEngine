#pragma once
#include "Sprite.h"
#include <memory>
#include <string>
#include <vector>

class Player;

/// <summary>
/// プレイヤーのHPをアイコンの並びで表示するUI。
/// </summary>
class HpUI {
public:
	/// <summary>
	/// デストラクタ。
	/// </summary>
	~HpUI() = default;
	/// <summary>
	/// 初期化処理。
	/// </summary>
	void Initialize(const std::string& frameTexturePath, const std::string& fillTexturePath, int maxHp);
	/// <summary>
	/// 更新処理。
	/// </summary>
	void Update(const Player* player);
	/// <summary>
	/// 描画処理。
	/// </summary>
	void Draw();

	/// <summary>
	/// 座標を設定する。
	/// </summary>
	void SetPosition(const TuboEngine::Math::Vector2& pos) {
		position_ = pos;
		alignRight_ = false;
	}
	/// <summary>
	/// 表示間隔を設定する。
	/// </summary>
	void SetSpacing(float spacing) { spacing_ = spacing; }
	/// <summary>
	/// スケールを設定する。
	/// </summary>
	void SetScale(float scale) { scale_ = scale; }
	/// <summary>
	/// AlignRight を設定する。
	/// </summary>
	void SetAlignRight(bool enable, float marginPx = 20.0f) {
		alignRight_ = enable;
		rightMargin_ = marginPx;
	}

private:
	// Per-HP sprites
	std::vector<std::unique_ptr<TuboEngine::Sprite>> frameSprites_;
	std::vector<std::unique_ptr<TuboEngine::Sprite>> fillSprites_;

	int maxHp_ = 0;
	int currentHp_ = 0;
	float animatedHp_ = 0.0f;  // animates from previous HP to current HP
	float shrinkSpeed_ = 4.0f; // icons per second shrink rate
	TuboEngine::Math::Vector2 position_{20.0f, 20.0f};
	float spacing_ = 36.0f; // pixel spacing
	float scale_ = 1.0f;    // uniform scale

	bool alignRight_ = false;
	float rightMargin_ = 20.0f;

	// cached paths (optional)
	std::string frameTex_;
	std::string fillTex_;
};