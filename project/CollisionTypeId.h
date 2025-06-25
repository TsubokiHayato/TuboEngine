#pragma once
#include <cstdint>

/// ---------- 識別IDの定義 ---------- ///
enum class CollisionTypeId : uint32_t {
	kPlayer,      // プレイヤーID
	kEnemy,  // 敵ID
	kWeapon,      // 武器ID
};