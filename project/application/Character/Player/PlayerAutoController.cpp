#include "PlayerAutoController.h"
#include "Player.h"
#include "Input.h"
#include "engine/graphic/Particle/ParticleManager.h"
#include "Character/Enemy/Enemy.h"

// Demo用の自動操作：
// - 一番近い敵に向かって移動
// - 一定条件で射撃フラグを立てる
void PlayerAutoController::Update(float dt) {
    if (!enabled_ || !owner_) {
        return;
    }

    Enemy* nearest = nullptr;
    float nearestDistSq = std::numeric_limits<float>::max();

    TuboEngine::Math::Vector3 playerPos = owner_->GetPosition();

    for (Enemy* e : enemies_) {
        if (!e || !e->GetIsAlive()) continue;
        TuboEngine::Math::Vector3 d = e->GetPosition() - playerPos;
        d.z = 0.0f;
        float distSq = d.x * d.x + d.y * d.y;
        if (distSq < nearestDistSq) {
            nearestDistSq = distSq;
            nearest = e;
        }
    }

    TuboEngine::Math::Vector3 moveDir{0.0f, 0.0f, 0.0f};
    bool wantShoot = false;

    if (nearest) {
        TuboEngine::Math::Vector3 toEnemy = nearest->GetPosition() - playerPos;
        toEnemy.z = 0.0f;
        float len = std::sqrt(toEnemy.x * toEnemy.x + toEnemy.y * toEnemy.y);
        if (len > 0.001f) {
            toEnemy.x /= len;
            toEnemy.y /= len;
        }

        float dist = std::sqrt(nearestDistSq);
        const float kDesiredDist = 10.0f;
        const float kStopBand    = 2.0f;

        if (dist > kDesiredDist + kStopBand) {
            moveDir = toEnemy;
        } else if (dist < kDesiredDist - kStopBand) {
            moveDir = {-toEnemy.x, -toEnemy.y, 0.0f};
        }

        if (dist <= kDesiredDist + kStopBand) {
            shootTimer_ += dt;
            if (shootTimer_ >= 0.5f) {
                shootTimer_ = 0.0f;
                wantShoot = true;
            }
        } else {
            shootTimer_ = 0.0f;
        }
    } else {
        moveDir = {0.0f, 0.0f, 0.0f};
        shootTimer_ = 0.0f;
    }

    owner_->SetAutoMoveDirection(moveDir);
    owner_->SetAutoShoot(wantShoot);
}
