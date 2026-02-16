#include "Weapon/PlayerWeapons.h"

#include <algorithm>
#include <cmath>

#include "Character/Player/Player.h"
#include "Input.h"
#include "Bullet/Player/PlayerBullet.h"

// ---- Bullet variants ----
class PiercingBullet final : public PlayerBullet {
public:
	bool ShouldDieOnEnemyHit() const override { return false; }
	// ここで貫通時の演出/追加ダメージ等を実装可能
	void OnHitEnemy(Collider* other) override { (void)other; }
};

// ---- Weapons ----
namespace {
	class BaseWeapon : public IWeapon {
	public:
		void Update(Player& /*player*/, float dt) override {
			if (cooldown_ > 0.0f) {
				cooldown_ = std::max(0.0f, cooldown_ - dt);
			}
		}

	protected:
		float cooldown_ = 0.0f;
		float fireInterval_ = 0.2f;
		float bulletSpeed_ = 0.0f;

		bool CanFire() const { return cooldown_ <= 0.0f; }
		void MarkFired() { cooldown_ = fireInterval_; }

		static TuboEngine::Math::Vector3 MakeDirFromAngle(float ang) {
			return {std::sin(ang), std::cos(ang), 0.0f};
		}
	};

	class NormalWeapon final : public BaseWeapon {
	public:
		NormalWeapon() {
			fireInterval_ = 0.2f;
			bulletSpeed_ = PlayerBullet::s_bulletSpeed;
		}
		const char* GetName() const override { return "Normal"; }
		void TryShoot(Player& player) override {
			if (!Input::GetInstance()->IsPressMouse(0) || !CanFire()) return;
			player.SpawnBullet<PlayerBullet>(0.0f, bulletSpeed_);
			MarkFired();
		}
	};

	class RapidWeapon final : public BaseWeapon {
	public:
		RapidWeapon() {
			fireInterval_ = 0.08f;
			bulletSpeed_ = PlayerBullet::s_bulletSpeed * 0.9f;
		}
		const char* GetName() const override { return "Rapid"; }
		void TryShoot(Player& player) override {
			if (!Input::GetInstance()->IsPressMouse(0) || !CanFire()) return;
			player.SpawnBullet<PlayerBullet>(0.0f, bulletSpeed_);
			MarkFired();
		}
	};

	class ShotgunWeapon final : public BaseWeapon {
	public:
		ShotgunWeapon() {
			fireInterval_ = 0.5f;
			bulletSpeed_ = PlayerBullet::s_bulletSpeed * 0.85f;
		}
		const char* GetName() const override { return "Shotgun"; }
		void TryShoot(Player& player) override {
			if (!Input::GetInstance()->IsPressMouse(0) || !CanFire()) return;
			constexpr int kPellets = 6;
			constexpr float kSpreadRad = 0.18f;
			for (int i = 0; i < kPellets; ++i) {
				float t = (kPellets == 1) ? 0.5f : (static_cast<float>(i) / static_cast<float>(kPellets - 1));
				float offset = (t - 0.5f) * 2.0f * kSpreadRad;
				player.SpawnBullet<PlayerBullet>(offset, bulletSpeed_);
			}
			MarkFired();
		}
	};

	class PiercingWeapon final : public BaseWeapon {
	public:
		PiercingWeapon() {
			fireInterval_ = 0.25f;
			bulletSpeed_ = PlayerBullet::s_bulletSpeed;
		}
		const char* GetName() const override { return "Piercing"; }
		void TryShoot(Player& player) override {
			if (!Input::GetInstance()->IsPressMouse(0) || !CanFire()) return;
			player.SpawnBullet<PiercingBullet>(0.0f, bulletSpeed_);
			MarkFired();
		}
	};
}

namespace PlayerWeapons {
	std::unique_ptr<IWeapon> CreateNormal() { return std::make_unique<NormalWeapon>(); }
	std::unique_ptr<IWeapon> CreateRapid() { return std::make_unique<RapidWeapon>(); }
	std::unique_ptr<IWeapon> CreateShotgun() { return std::make_unique<ShotgunWeapon>(); }
	std::unique_ptr<IWeapon> CreatePiercing() { return std::make_unique<PiercingWeapon>(); }
}
