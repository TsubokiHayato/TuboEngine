#pragma once

#include "Bullet/Player/PlayerBullet.h"

///--------------------------------------------------
/// @brief ショットガン散弾用の弾。
///
/// @details
/// `PlayerBullet` は `Update()` 内で毎フレーム `playerRotation.z` から速度を再計算します。
/// 散弾では弾ごとに異なる角度（拡散）を維持したいので、生成時に設定した回転を固定保持し、
/// `Update()` の直前に注入することで角度が揃ってしまう問題を回避します。
///--------------------------------------------------
class ShotgunPelletBullet final : public PlayerBullet {
public:
	// Player 生成側から呼ばれる想定（SpawnBulletWithRotationZなど）
	void SetPlayerRotation(const TuboEngine::Math::Vector3& rotation) {
		PlayerBullet::SetPlayerRotation(rotation);
		fixedPlayerRotation_ = rotation;
		fixedSet_ = true;
	}

	void Update() override {
		if (fixedSet_) {
			PlayerBullet::SetPlayerRotation(fixedPlayerRotation_);
		}
		PlayerBullet::Update();
	}

private:
	TuboEngine::Math::Vector3 fixedPlayerRotation_{};
	bool fixedSet_ = false;
};
