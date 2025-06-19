#include "CollisionManager.h"
#include "Vector3.h"
#include <cmath>

bool CollisionManager::CheckPlayerEnemyCollision(const Player& player, const Enemy& enemy, float playerRadius, float enemyRadius) {
	Vector3 pPos = player.GetPosition();
	Vector3 ePos = enemy.GetPosition();
	float distSq = Vector3::DistanceSquared(pPos, ePos);
	float radiusSum = playerRadius + enemyRadius;
	return distSq <= (radiusSum * radiusSum);
}

void CollisionManager::CheckPlayerEnemiesCollision(Player& player, std::vector<std::unique_ptr<Enemy>>& enemies, float playerRadius, float enemyRadius) {
	for (auto& enemy : enemies) {
		if (enemy->GetIsAlive() && CheckPlayerEnemyCollision(player, *enemy, playerRadius, enemyRadius)) {
			// ここで当たり判定時の処理を記述
			player.TakeDamage(10); // 例: プレイヤーにダメージ
			enemy->TakeDamage(10); // 例: 敵にもダメージ
		}
	}
}
