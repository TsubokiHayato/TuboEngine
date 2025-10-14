#include "Block/Block.h"
#include "Character/Enemy/Enemy.h"
#include "Character/Player/Player.h"
#include "DropWaveAnimation.h"
#include <algorithm>
#include <cmath>

DropWaveAnimation::DropWaveAnimation() {}

void DropWaveAnimation::Initialize(const std::vector<Vector3>& blockTargets, const Vector3& playerTarget, const std::vector<Vector3>& enemyTargets, int playerMapX, int playerMapY) {
	blockTargetPositions_ = blockTargets;
	playerTargetPosition_ = playerTarget;
	enemyTargetPositions_ = enemyTargets;

	blockLayers_.clear();
	enemyLayers_.clear();
	maxLayer_ = 0;

	// ブロックのレイヤー計算（チェビシェフ距離）
	for (const auto& pos : blockTargets) {
		int x = static_cast<int>(std::round(pos.x));
		int y = static_cast<int>(std::round(pos.y));
		int layer = std::max(std::abs(x - playerMapX), std::abs(y - playerMapY));
		blockLayers_.push_back(layer);
		maxLayer_ = std::max(maxLayer_, layer);
	}
	// エネミーのレイヤー計算
	for (const auto& pos : enemyTargets) {
		int x = static_cast<int>(std::round(pos.x));
		int y = static_cast<int>(std::round(pos.y));
		int layer = std::max(std::abs(x - playerMapX), std::abs(y - playerMapY));
		enemyLayers_.push_back(layer);
		maxLayer_ = std::max(maxLayer_, layer);
	}
	playerLayer_ = 0;

	currentLayer_ = 0;
	layerTimer_ = 0.0f;
	isFinished_ = false;
}

void DropWaveAnimation::Update(float deltaTime) {
	if (isFinished_)
		return;
	layerTimer_ += deltaTime;

	if (layerTimer_ >= kDropDuration_) {
		++currentLayer_;
		layerTimer_ = 0.0f;
		if (currentLayer_ > maxLayer_) {
			isFinished_ = true;
		}
	}
}

void DropWaveAnimation::ApplyPositions(std::vector<std::unique_ptr<Block>>& blocks, Player* player, std::vector<std::unique_ptr<Enemy>>& enemies) {
	auto lerp = [](float a, float b, float t) { return a + (b - a) * t; };

	// ブロック
	for (size_t i = 0; i < blocks.size(); ++i) {
		int layer = blockLayers_[i];
		if (layer < currentLayer_) {
			blocks[i]->SetPosition(blockTargetPositions_[i]);
		} else if (layer == currentLayer_) {
			float t = std::min(layerTimer_ / kDropDuration_, 1.0f);
			Vector3 start = blockTargetPositions_[i];
			start.z += kDropOffsetZ_;
			Vector3 pos;
			pos.x = lerp(start.x, blockTargetPositions_[i].x, t);
			pos.y = lerp(start.y, blockTargetPositions_[i].y, t);
			pos.z = lerp(start.z, blockTargetPositions_[i].z, t);
			blocks[i]->SetPosition(pos);
		} else {
			Vector3 pos = blockTargetPositions_[i];
			pos.z += kDropOffsetZ_;
			blocks[i]->SetPosition(pos);
		}
	}

	// プレイヤー
	if (currentLayer_ > 0) {
		player->SetPosition(playerTargetPosition_);
	} else {
		float t = std::min(layerTimer_ / kDropDuration_, 1.0f);
		Vector3 start = playerTargetPosition_;
		start.z += kDropOffsetZ_;
		Vector3 pos;
		pos.x = lerp(start.x, playerTargetPosition_.x, t);
		pos.y = lerp(start.y, playerTargetPosition_.y, t);
		pos.z = lerp(start.z, playerTargetPosition_.z, t);
		player->SetPosition(pos);
	}

	// エネミー
	for (size_t i = 0; i < enemies.size(); ++i) {
		int layer = enemyLayers_[i];
		if (layer < currentLayer_) {
			enemies[i]->SetPosition(enemyTargetPositions_[i]);
		} else if (layer == currentLayer_) {
			float t = std::min(layerTimer_ / kDropDuration_, 1.0f);
			Vector3 start = enemyTargetPositions_[i];
			start.z += kDropOffsetZ_;
			Vector3 pos;
			pos.x = lerp(start.x, enemyTargetPositions_[i].x, t);
			pos.y = lerp(start.y, enemyTargetPositions_[i].y, t);
			pos.z = lerp(start.z, enemyTargetPositions_[i].z, t);
			enemies[i]->SetPosition(pos);
		} else {
			Vector3 pos = enemyTargetPositions_[i];
			pos.z += kDropOffsetZ_;
			enemies[i]->SetPosition(pos);
		}
	}
}

void DropWaveAnimation::Skip() {
	currentLayer_ = maxLayer_ + 1;
	isFinished_ = true;
}