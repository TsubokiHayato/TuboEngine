#pragma once

#include "Bullet/Player/PlayerBullet.h"

///--------------------------------------------------
/// @brief 貫通弾。
///--------------------------------------------------
class PiercingBullet final : public PlayerBullet {
public:
	bool ShouldDieOnEnemyHit() const override { return false; }
	void OnHitEnemy(Collider* other) override { (void)other; }

	static void DrawImGuiGlobal() {
		// 現状は固有パラメータ無し。将来のダメージ倍率等を追加する場所。
	}
};
