#pragma once

#include <memory>

#include "Sprite.h"

class Player;
class Camera;

/// @brief プレイヤー周囲に表示する「円形UI」。
/// - 外周: HPリング
/// - 内周: Dodge(スタミナ代替)リング
class PlayerStatusRingUI {
public:
	void Initialize();
	void Update(const Player* player, const Camera* cam);
	void Draw();

	// 位置オフセット（スクリーン座標, px）
	void SetScreenOffset(const TuboEngine::Math::Vector2& offsetPx) { screenOffsetPx_ = offsetPx; }

private:
	static TuboEngine::Math::Vector2 WorldToScreen_(const TuboEngine::Math::Vector3& world, const Camera* cam);
	static float Clamp01_(float v);

private:
	std::unique_ptr<Sprite> hpRing_;
	std::unique_ptr<Sprite> dodgeRing_;

	// テクスチャ
	const char* hpTex_ = "UI/ring_hp.png";
	const char* dodgeTex_ = "UI/ring_dodge.png";

	// スクリーンサイズ/オフセット
	TuboEngine::Math::Vector2 screenOffsetPx_{0.0f, -60.0f}; // プレイヤーの少し上

	// 見た目
	float hpRingSizePx_ = 96.0f;
	float dodgeRingSizePx_ = 72.0f;
	float ringAlpha_ = 0.85f;

	// HPの最大値（現状Player側にMaxHPが無い想定なので固定。必要なら後でPlayerに追加）
	int assumedMaxHp_ = 5;
};
