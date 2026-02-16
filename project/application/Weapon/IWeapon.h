#pragma once
#include <memory>

class Player;

/// @brief Player用の武器インターフェース（Strategy）
class IWeapon {
public:
	virtual ~IWeapon() = default;
	virtual const char* GetName() const = 0;

	/// @brief 1フレーム更新（クールダウンなど）
	virtual void Update(Player& player, float dt) = 0;

	/// @brief 射撃入力を処理して弾を生成する
	virtual void TryShoot(Player& player) = 0;
};
