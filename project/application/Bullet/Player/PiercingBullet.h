#pragma once

#include "Bullet/Player/PlayerBullet.h"

///--------------------------------------------------
/// @brief 貫通弾。
///--------------------------------------------------
class PiercingBullet final : public PlayerBullet {
public:
	bool ShouldDieOnEnemyHit() const override { return false; }
	void OnHitEnemy(Collider* other) override { (void)other; }
};
