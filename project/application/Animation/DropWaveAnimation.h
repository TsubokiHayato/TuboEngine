#pragma once
#include "Vector3.h"
#include <chrono>
#include <memory>
#include <vector>

class Block;
class Enemy;
class Player;

/// <summary>
/// 波紋状にオブジェクトを落下させるアニメーション制御クラス
/// </summary>
class DropWaveAnimation {
public:
	DropWaveAnimation();
	void Initialize(const std::vector<Vector3>& blockTargets, const Vector3& playerTarget, const std::vector<Vector3>& enemyTargets, int playerMapX, int playerMapY);
	void Update(float deltaTime);
	void ApplyPositions(std::vector<std::unique_ptr<Block>>& blocks, Player* player, std::vector<std::unique_ptr<Enemy>>& enemies);
	bool IsFinished() const { return isFinished_; }
	void Skip();

private:
	// アニメーション定数
	static constexpr float kDropDuration_ = 0.5f;
	static constexpr float kDropOffsetZ_ = 30.0f;

	// 各オブジェクトの目標座標
	std::vector<Vector3> blockTargetPositions_;
	Vector3 playerTargetPosition_;
	std::vector<Vector3> enemyTargetPositions_;

	// 各オブジェクトのレイヤー番号
	std::vector<int> blockLayers_;
	int playerLayer_ = 0;
	std::vector<int> enemyLayers_;

	// 進行管理
	int currentLayer_ = 0;
	float layerTimer_ = 0.0f;
	bool isFinished_ = false;

	// 最大レイヤー
	int maxLayer_ = 0;
};