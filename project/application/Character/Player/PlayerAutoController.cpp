// application/Character/Player/PlayerAutoController.cpp
#include "PlayerAutoController.h"
#include "Character/Enemy/Enemy.h"
#include "Input.h"
#include "Player.h"
#include "engine/graphic/Particle/ParticleManager.h"

// Demo用の自動操作：
// - 一番近い敵に向かって移動
// - 一定条件で射撃フラグを立てる
// - 危険距離ではプレイヤーのダッシュ(回避)を使って離脱
void PlayerAutoController::Update(float dt) {
	if (!enabled_ || !owner_) {
		return;
	}

	// 危険距離のしきい値（突進を食らいたくない距離）
	static const float kDangerDist = 4.0f;
	// 近づきたい目安距離
	static const float kApproachDist = 12.0f;

	Enemy* nearest = nullptr;
	float nearestDistSq = std::numeric_limits<float>::max();

	TuboEngine::Math::Vector3 playerPos = owner_->GetPosition();

	for (Enemy* e : enemies_) {
		if (!e || !e->GetIsAlive()) {
			continue;
		}
		TuboEngine::Math::Vector3 d = e->GetPosition() - playerPos;
		d.z = 0.0f;
		float distSq = d.x * d.x + d.y * d.y;
		if (distSq < nearestDistSq) {
			nearestDistSq = distSq;
			nearest = e;
		}
	}

	TuboEngine::Math::Vector3 moveDir{0.0f, 0.0f, 0.0f};
	TuboEngine::Math::Vector3 aimDir{0.0f, -1.0f, 0.0f}; // デフォルト
	bool wantShoot = false;

	if (nearest) {
		TuboEngine::Math::Vector3 toEnemy = nearest->GetPosition() - playerPos;
		toEnemy.z = 0.0f;
		float len = std::sqrt(toEnemy.x * toEnemy.x + toEnemy.y * toEnemy.y);
		TuboEngine::Math::Vector3 dirNorm{0.0f, 0.0f, 0.0f};
		if (len > 0.001f) {
			dirNorm.x = toEnemy.x / len;
			dirNorm.y = toEnemy.y / len;
		}

		float dist = std::sqrt(nearestDistSq);

		// 常に最近敵の方向を向く
		aimDir = dirNorm;

		// 危険距離内なら、プレイヤーのダッシュ（回避）を使って離脱
		if (dist < kDangerDist && owner_->CanDodge()) {
			// 敵と逆方向へダッシュするように、入力ベクトルを設定
			// Player::GetDodgeInputDirection() はキーボード入力を読むので、
			// ここでは「擬似入力」として owner 側に別のAPIを用意しても良いですが、
			// 手早くやるなら「向きに応じて既存の StartDodge をそのまま使い、
			// rotation は AutoAimDir 経由で敵と逆向きになるので OK」という運用もありです。
			// ここでは単純に StartDodge を呼ぶだけにします。

			owner_->AutoStartDodge(); // 既存のダッシュ実装を使用
			// 移動入力はこのフレームは0（ダッシュ側が実際の移動を行う）
			moveDir = {0.0f, 0.0f, 0.0f};
		} else {
			// 通常時: 遠ければ寄る、それ以外はその場で戦う
			if (dist > kApproachDist) {
				moveDir = dirNorm; // 敵方向へ前進
			} else {
				moveDir = {0.0f, 0.0f, 0.0f}; // 攻撃レンジ内: 基本その場
			}

			// 攻撃距離には入ってよいので、ApproachDist 以内なら射撃
			if (dist <= kApproachDist) {
				shootTimer_ += dt;
				if (shootTimer_ >= 0.5f) {
					shootTimer_ = 0.0f;
					wantShoot = true;
				}
			} else {
				shootTimer_ = 0.0f;
			}
		}
	} else {
		moveDir = {0.0f, 0.0f, 0.0f};
		shootTimer_ = 0.0f;
	}

	owner_->SetAutoMoveDirection(moveDir);
	owner_->SetAutoShoot(wantShoot);
	owner_->SetAutoAimDirection(aimDir);
}