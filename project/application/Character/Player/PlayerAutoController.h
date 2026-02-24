#pragma once
#include "engine/math/Vector3.h"
#include <vector>

class Player;
class Enemy;

// プレイヤーの自動操作を行う簡易AIコントローラ
class PlayerAutoController {
public:
	void Initialize(Player* owner) { owner_ = owner; }

	void SetEnabled(bool enabled) { enabled_ = enabled; }
	bool IsEnabled() const { return enabled_; }

	// Demoモード時の更新。必要に応じて移動・射撃などの指示を出す
	void Update(float dt);

	// 外部（Stateなど）から「このフレームで見える敵リスト」を渡す
	void SetEnemyList(const std::vector<Enemy*>& enemies) { enemies_ = enemies; }

private:
	Player* owner_ = nullptr;
	bool enabled_ = false;

	// 簡易パターン用のタイマー
	float moveTimer_ = 0.0f;
	float shootTimer_ = 0.0f;

	std::vector<Enemy*> enemies_;

	// ★ ブリンク用パラメータ
	float blinkCooldown_ = 2.0f;     // ブリンクのクールダウン(秒)
	float blinkCooldownTimer_ = 0.0f;
	float blinkDistance_ = 8.0f;     // 一回のブリンク距離
	float blinkDangerDist_ = 5.0f;   // この距離より近いと「危険」とみなす
};