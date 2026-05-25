// application/Character/Player/PlayerAutoController.cpp
#include "PlayerAutoController.h"
#include "Character/Enemy/Enemy.h"
#include "Player.h"
#include <cmath>
#include <random>

// ─── 内部ユーティリティ ───────────────────────────────────────────────────────

namespace {
	// 簡易乱数 (スレッドローカル。スレッド安全性は不要)
	static std::mt19937 s_rng{std::random_device{}()};

	float RandFloat(float lo, float hi) {
		std::uniform_real_distribution<float> dist(lo, hi);
		return dist(s_rng);
	}

	// 2Dベクトルの長さ
	float Len2(float x, float y) { return std::sqrt(x * x + y * y); }

	// 2D右直交ベクトル (x, y) → (-y, x)
	void Perp(float x, float y, float& outX, float& outY) {
		outX = -y;
		outY = x;
	}
}

// ─── 定数 ────────────────────────────────────────────────────────────────────

static constexpr float kIdealDist     = 9.0f;  // 理想的な対敵距離
static constexpr float kDangerDist    = 4.5f;  // これより近いと「危険」
static constexpr float kFarDist       = 14.0f; // これより遠いと近づく
static constexpr float kStrafeWeight  = 0.6f;  // ストラフの強さ（0=純接近、1=純横移動）
static constexpr float kApproachWeight = 0.5f; // 接近/後退の強さ

// ─── 更新 ────────────────────────────────────────────────────────────────────

void PlayerAutoController::Update(float dt) {
	if (!enabled_ || !owner_) {
		return;
	}

	// ── タイマー更新 ──────────────────────────────────────────────────────────
	dodgeCooldownTimer_ = std::max(0.0f, dodgeCooldownTimer_ - dt);
	strafeChangeTimer_  = std::max(0.0f, strafeChangeTimer_ - dt);
	wanderChangeTimer_  = std::max(0.0f, wanderChangeTimer_ - dt);

	// ── 最近敵を検索 ──────────────────────────────────────────────────────────
	const TuboEngine::Math::Vector3 playerPos = owner_->GetPosition();

	Enemy* nearest = nullptr;
	float nearestDist = std::numeric_limits<float>::max();

	for (Enemy* e : enemies_) {
		if (!e || !e->GetIsAlive()) continue;
		TuboEngine::Math::Vector3 d = e->GetPosition() - playerPos;
		d.z = 0.0f;
		float dist = Len2(d.x, d.y);
		if (dist < nearestDist) {
			nearestDist = dist;
			nearest = e;
		}
	}

	// ── 移動・エイム・射撃の決定 ─────────────────────────────────────────────
	TuboEngine::Math::Vector3 moveDir{0.0f, 0.0f, 0.0f};
	TuboEngine::Math::Vector3 aimDir{0.0f, -1.0f, 0.0f}; // デフォルト
	bool wantShoot = false;

	if (nearest) {
		// ─ 敵への方向 ─────────────────────────────────────────────────────────
		float dx = nearest->GetPosition().x - playerPos.x;
		float dy = nearest->GetPosition().y - playerPos.y;
		float dist = Len2(dx, dy);

		float toEx = 0.0f, toEy = 0.0f; // 敵方向正規化ベクトル
		if (dist > 0.001f) {
			toEx = dx / dist;
			toEy = dy / dist;
		}

		// ─ エイム（目標方向を設定するだけ。補間は Player::Update 内で行う） ──
		aimDir.x = toEx;
		aimDir.y = toEy;

		// ─ 危険距離なら即回避（敵と逆方向へ） ───────────────────────────────
		if (dist < kDangerDist && dodgeCooldownTimer_ <= 0.0f && owner_->CanDodge()) {
			// 敵から逃げる方向 + ランダム横成分で「ただ後退」を避ける
			float escX = -toEx + RandFloat(-0.4f, 0.4f);
			float escY = -toEy + RandFloat(-0.4f, 0.4f);
			float escLen = Len2(escX, escY);
			if (escLen > 0.001f) {
				escX /= escLen;
				escY /= escLen;
			}
			TuboEngine::Math::Vector3 escDir{escX, escY, 0.0f};
			owner_->AutoStartDodgeDir(escDir);
			dodgeCooldownTimer_ = kDodgeCooldown;
			// 回避中は移動入力を回避方向へ合わせる（慣性ブレンド）
			moveDir = escDir;
		} else {
			// ─ ストラフ方向の管理 ─────────────────────────────────────────────
			if (strafeChangeTimer_ <= 0.0f) {
				// 1.5〜3.5秒ごとにランダムに方向転換
				strafeDirSign_ = (RandFloat(0.0f, 1.0f) > 0.5f) ? 1.0f : -1.0f;
				strafeChangeTimer_ = RandFloat(1.5f, 3.5f);
			}

			// ─ 横方向ベクトル（敵の右/左） ───────────────────────────────────
			float perpX, perpY;
			Perp(toEx, toEy, perpX, perpY);
			perpX *= strafeDirSign_;
			perpY *= strafeDirSign_;

			// ─ 前後（接近/後退）方向を距離で決定 ────────────────────────────
			float radialX = 0.0f, radialY = 0.0f;
			if (dist > kFarDist) {
				// 遠すぎる → 近づく
				radialX = toEx;
				radialY = toEy;
			} else if (dist < kIdealDist * 0.8f) {
				// 少し近い → 後退
				radialX = -toEx;
				radialY = -toEy;
			}
			// kIdealDist 付近はほぼ横移動のみ

			// ─ ブレンド（ストラフ + 前後） ────────────────────────────────────
			float blendX = perpX * kStrafeWeight + radialX * kApproachWeight;
			float blendY = perpY * kStrafeWeight + radialY * kApproachWeight;
			float blendLen = Len2(blendX, blendY);
			if (blendLen > 0.001f) {
				moveDir.x = blendX / blendLen;
				moveDir.y = blendY / blendLen;
			}

			// ─ 射撃（理想距離内にいる間、タイマーで発射） ───────────────────
			if (dist <= kFarDist) {
				shootTimer_ -= dt;
				if (shootTimer_ <= 0.0f) {
					wantShoot = true;
					// 間隔をランダムに変動させる（リズムを壊す）
					shootInterval_ = RandFloat(0.3f, 0.65f);
					shootTimer_ = shootInterval_;
				}
			} else {
				shootTimer_ = 0.0f;
			}
		}
	} else {
		// ─ 敵なし: ゆっくり巡回 ──────────────────────────────────────────────
		if (wanderChangeTimer_ <= 0.0f) {
			// 2〜4秒ごとにランダムな角度へ方向転換
			wanderAngle_ += RandFloat(-1.2f, 1.2f); // ±70度程度
			wanderChangeTimer_ = RandFloat(2.0f, 4.0f);
		}
		// 巡回速度を少し落とす（moveDir の長さを調整）
		moveDir.x = std::cos(wanderAngle_) * 0.6f;
		moveDir.y = std::sin(wanderAngle_) * 0.6f;
		// 射撃タイマーはリセット
		shootTimer_ = 0.0f;
	}

	// ── コントローラへ渡す ────────────────────────────────────────────────────
	owner_->SetAutoMoveDirection(moveDir);
	owner_->SetAutoShoot(wantShoot);
	owner_->SetAutoAimDirection(aimDir);
}