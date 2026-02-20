#pragma once
#include <memory>
#include <vector>
#define NOMINMAX
#define NOMINMIN
#include <DirectXMath.h>

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
	// Dodge gauge: セグメントを複数枚回して円弧にする
	std::vector<std::unique_ptr<Sprite>> dodgeSegments_;

	// テクスチャ
	const char* hpTex_ = "Hp.png";
	// 1枚の細い白い矩形（またはグラデ）を用意して、回転・着色して使う
	const char* dodgeSegTex_ = "barrier.png";

	// スクリーンサイズ/オフセット
	TuboEngine::Math::Vector2 screenOffsetPx_{0.0f, 0.0f}; // プレイヤー中心

	// 見た目
	float hpRingSizePx_ = 96.0f;
	float dodgeRingSizePx_ = 72.0f;
	float ringAlpha_ = 0.85f;

	// Dodge gauge segments
	int dodgeSegmentCount_ = 48;           // 分割数（多いほど滑らか）
	float dodgeSegmentThicknessPx_ = 6.0f; // セグメントの短辺
	float dodgeSegmentLengthPx_ = 14.0f;   // セグメントの長辺（リングの太さ感）
	float dodgeStartAngleRad_ = -DirectX::XM_PIDIV2; // 12時開始

	// HPの最大値（現状Player側にMaxHPが無い想定なので固定。必要なら後でPlayerに追加）
	int assumedMaxHp_ = 5;
};
