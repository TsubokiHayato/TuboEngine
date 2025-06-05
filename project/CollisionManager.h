#pragma once
#include "Enemy.h"
#include "Player.h"
#include <memory>
#include <vector>

class CollisionManager {
public:
	// プレイヤーと敵の当たり判定をチェック
	static bool CheckPlayerEnemyCollision(const Player& player, const Enemy& enemy, float playerRadius, float enemyRadius);

	// プレイヤーと複数の敵の当たり判定
	static void CheckPlayerEnemiesCollision(Player& player, std::vector<std::unique_ptr<Enemy>>& enemies, float playerRadius, float enemyRadius);
};
