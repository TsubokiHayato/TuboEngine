#include "Weapon/PlayerWeapons.h"

#include <algorithm>
#include <cmath>

#include "Character/Player/Player.h"
#include "Input.h"
#include "Bullet/Player/PlayerBullet.h"

// Bullet variants (split into dedicated headers)
#include "Bullet/Player/ShotgunPelletBullet.h"
#include "Bullet/Player/PiercingBullet.h"

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
			// 現状の弾実装は「毎フレーム playerRotation.z から方向を作り直す」ため、
			// そのまま offset を付けても全弾同じ方向に揃って見えることがある。
			// 散弾は弾に固定の角度を持たせて撃つ。
			const float baseRotZ = player.GetRotation().z;
			for (int i = 0; i < kPellets; ++i) {
				float t = (kPellets == 1) ? 0.5f : (static_cast<float>(i) / static_cast<float>(kPellets - 1));
				float offset = (t - 0.5f) * 2.0f * kSpreadRad;
				player.SpawnBulletWithRotationZ<ShotgunPelletBullet>(baseRotZ + offset, 0.0f, bulletSpeed_);
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
	std::unique_ptr<IWeapon> CreateShotgun() { return std::make_unique<ShotgunWeapon>(); }
	std::unique_ptr<IWeapon> CreatePiercing() { return std::make_unique<PiercingWeapon>(); }
}
