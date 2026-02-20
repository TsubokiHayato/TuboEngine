#pragma once
#include <memory>

#include "Weapon/IWeapon.h"

class Player;

namespace PlayerWeapons {
	std::unique_ptr<IWeapon> CreateNormal();
	std::unique_ptr<IWeapon> CreateShotgun();
	std::unique_ptr<IWeapon> CreatePiercing();
}
