#pragma once
#include "engine/math/Vector3.h"
#include <vector>

class Player;
class Enemy;

// プレイヤーの自動操作を行う簡易AIコントローラ
// より自然な立ち回りを実現するため以下を実装:
//  - エイムは補間済み (Player::Update内で lerp)
//  - 移動はストラフ（横移動）を交えた距離キープ
//  - 回避は「敵から逃げる方向」を計算してから発動
//  - 敵なし時はゆっくり巡回
//  - 適度なランダム性で機械的に見えないよう配慮
/// <summary>
/// デモプレイ用にプレイヤーを自動操作するコントローラ。適度なランダム性で機械的に見えないように動かす。
/// </summary>
class PlayerAutoController {
public:
	/// <summary>
	/// 初期化処理。
	/// </summary>
	void Initialize(Player* owner) { owner_ = owner; }

	/// <summary>
	/// Enabled の取得・設定。
	/// </summary>
	void SetEnabled(bool enabled) { enabled_ = enabled; }
	bool IsEnabled() const { return enabled_; }

	// Demoモード時の更新。必要に応じて移動・射撃などの指示を出す
	void Update(float dt);

	// 外部（Stateなど）から「このフレームで見える敵リスト」を渡す
	void SetEnemyList(const std::vector<Enemy*>& enemies) { enemies_ = enemies; }
	const std::vector<Enemy*>& GetEnemyList() const { return enemies_; }

private:
	Player* owner_ = nullptr;
	bool enabled_ = false;

	// 敵リスト
	std::vector<Enemy*> enemies_;

	// ─── 回避関連 ───
	float dodgeCooldownTimer_ = 0.0f;     // 回避再使用クールダウン
	static constexpr float kDodgeCooldown = 1.5f; // 回避インターバル（秒）

	// ─── 射撃関連 ───
	float shootTimer_ = 0.0f;
	float shootInterval_ = 0.4f;           // 次弾発射まで（ランダム変動）

	// ─── ストラフ（横移動）関連 ───
	float strafeDirSign_ = 1.0f;           // +1 or -1 (どちらに横移動するか)
	float strafeChangeTimer_ = 0.0f;       // ストラフ方向転換タイマー

	// ─── 巡回（敵なし時）関連 ───
	float wanderAngle_ = 0.0f;             // 巡回方向角度
	float wanderChangeTimer_ = 0.0f;       // 巡回方向変化タイマー
};