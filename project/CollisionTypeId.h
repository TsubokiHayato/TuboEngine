#pragma once
#include <cstdint>

/// ---------- 識別IDの定義 ---------- ///
enum class CollisionTypeId : uint32_t {
	kPlayer,       // プレイヤーID
	kEnemy,        // 敵ID
	kPlayerWeapon, // プレイヤーの武器ID
	kEnemyWeapon   // エネミーの武器ID
};