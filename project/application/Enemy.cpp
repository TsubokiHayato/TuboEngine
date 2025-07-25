#include "Enemy.h"
#include "CollisionTypeId.h"
#include "Enemy.h"
#include "ImGuiManager.h"
#include "TextureManager.h"
#include "Player.h"
#include <cmath>

constexpr float kPI = 3.14159265358979323846f;

Enemy::Enemy() {}
Enemy::~Enemy() {}

void Enemy::Initialize() {
	// プレイヤーのコライダーの設定
	Collider::SetTypeID(static_cast<uint32_t>(CollisionTypeId::kEnemy));

	// Enemy固有の初期化
	position = {0.0f, 0.0f, 5.0f};
	rotation = {};
	scale = {1.0f, 1.0f, 1.0f}; // 初期スケール

	object3d = std::make_unique<Object3d>();
	const std::string modelFileNamePath = "barrier.obj";
	object3d->Initialize( modelFileNamePath);
	object3d->SetPosition(position);
	object3d->SetRotation(rotation);
	object3d->SetScale(scale);



	std::string particleTextureHandle = "gradationLine.png";
	TextureManager::GetInstance()->LoadTexture(particleTextureHandle);

	// パーティクル関連は初期化しない（ヒット時に生成）
	particle = nullptr;
	particleEmitter_ = nullptr;
}


// 角度差分を[-π, π]に正規化する関数
static float NormalizeAngle(float angle) {
	while (angle > kPI) angle -= 2.0f * kPI;
	while (angle < -kPI) angle += 2.0f * kPI;
	return angle;
}
void Enemy::Update() {
	// プレイヤーがいれば距離と方向を計算
	float distanceToPlayer = 0.0f;
	Vector3 toPlayer = {0, 0, 0};
	if (player_) {
		toPlayer = player_->GetPosition() - position;
		distanceToPlayer = std::sqrt(toPlayer.x * toPlayer.x + toPlayer.y * toPlayer.y + toPlayer.z * toPlayer.z);
	}

	// --- 視覚ギミック追加 ---
	bool canSeePlayer = CanSeePlayer();

	// 状態遷移
	if (player_) {
		if (!canSeePlayer) {
			state_ = State::Idle; // 見えなければ待機
		} else if (distanceToPlayer > moveStartDistance_) {
			state_ = State::Idle; // 一定以上離れていると待機
		} else if (distanceToPlayer > shootDistance_) {
			state_ = State::Move; // 移動範囲内
		} else {
			state_ = State::Shoot; // 射撃範囲内
		}
	}
	// プレイヤーの方向を向く（一定速度で回転）
	if (player_ && state_ != State::Idle) {
		float angleZ = std::atan2(toPlayer.y, toPlayer.x);
		float diff = NormalizeAngle(angleZ - rotation.z);
		float maxTurn = turnSpeed_;
		if (std::fabs(diff) < maxTurn) {
			rotation.z = angleZ;
		} else {
			rotation.z += (diff > 0 ? 1 : -1) * maxTurn;
			rotation.z = NormalizeAngle(rotation.z);
		}
	}

	// 状態ごとの行動
	switch (state_) {
	case State::Idle:
		// 何もしない
		break;
	case State::Move:
		if (player_) {
			Vector3 dir = toPlayer;
			dir.z = 0.0f;
			float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
			if (len > 0.001f) {
				dir.x /= len;
				dir.y /= len;
				position.x += dir.x * moveSpeed_;
				position.y += dir.y * moveSpeed_;
			}
		}
		break;
	case State::Shoot: {
		static float bulletTimer = 0.0f;
		bulletTimer += 1.0f / 60.0f;
		if (bulletTimer >= 1.0f) {
			bulletTimer = 0.0f;
			if (player_) {
				bullet = std::make_unique<EnemyNormalBullet>();
				bullet->Initialize(position);
				bullet->SetEnemyPosition(position);
				bullet->SetEnemyRotation(rotation);
				bullet->SetPlayer(player_);
				bullet->SetCamera(camera_);
			}
		}
		if (bullet) {
			bullet->Update();
		}
	} break;
	}

	object3d->SetPosition(position);
	object3d->SetRotation(rotation);
	object3d->SetScale(scale);
	object3d->SetCamera(camera_);
	object3d->Update();

	if (!wasHit && isHit) {
		EmitHitParticle();
	}
	isHit = false;
	wasHit = isHit;

	if (particle) {
		particle->SetCamera(camera_);
		particle->Update();
	}
	if (particleEmitter_) {
		particleEmitter_->Update();
	}
}

void Enemy::EmitHitParticle() {
	// 既存のパーティクルをクリア
	particle = nullptr;
	particleEmitter_ = nullptr;

	std::string particleTextureHandle = "gradationLine.png";
	TextureManager::GetInstance()->LoadTexture(particleTextureHandle);

	particle = std::make_unique<Particle>();
	particle->Initialize(ParticleType::Ring);
	particle->CreateParticleGroup("Particle", particleTextureHandle);

	// 必ず最新のpositionを使う
	particleTranslate = {
	    {1.0f, 1.0f, 1.0f},
        {0.0f, 0.0f, 0.0f},
        position
    };
	particleVelocity = {};
	particleColor = {1.0f, 1.0f, 1.0f, 1.0f};
	particleLifeTime = 1.0f;
	particleCurrentTime = 0.0f;

	particleEmitter_ = std::make_unique<ParticleEmitter>(particle.get(), "Particle", particleTranslate, particleVelocity, particleColor, particleLifeTime, particleCurrentTime, 1, 1.0f, false);
}


void Enemy::Draw() {
	if (object3d) {
		object3d->Draw();
	}
	if (bullet) {
		bullet->Draw();
	}
}

void Enemy::ParticleDraw() {
	if (particle) {
		particle->Draw();
	}
}

void Enemy::DrawImGui() {
	ImGui::Begin("Enemy");
	ImGui::Text("Enemy Position: (%.2f, %.2f, %.2f)", position.x, position.y, position.z);
	ImGui::Text("Enemy Velocity: (%.2f, %.2f, %.2f)", velocity.x, velocity.y, velocity.z);
	ImGui::Text("Enemy HP: %d", HP);
	ImGui::Text("Enemy Alive: %s", isAlive ? "Yes" : "No");
	ImGui::Text("Hit: %s", isHit ? "Yes" : "No");
	ImGui::Text("wasHit: %s", isHit ? "Yes" : "No");
	ImGui::SliderFloat("Turn Speed", &turnSpeed_, 0.01f, 1.0f, "%.2f");
	ImGui::End();
}

void Enemy::Move() {}

bool Enemy::CanSeePlayer() {
	if (!player_ || !mapChipField)
		return false;

	Vector3 from = position;
	Vector3 to = player_->GetPosition();

	// XY平面で判定（Zは無視）
	Vector2 start = {from.x, from.y};
	Vector2 end = {to.x, to.y};

	// 線分上を一定間隔でサンプリング
	const float step = 0.5f; // サンプリング間隔（ブロックサイズより小さめ推奨）
	Vector2 dir = end - start;
	float length = std::sqrt(dir.x * dir.x + dir.y * dir.y);
	if (length < 0.001f)
		return true; // 同じ位置なら見える

	dir.x /= length;
	dir.y /= length;

	for (float t = 0.0f; t <= length; t += step) {
		Vector2 pos = start + dir * t;
		Vector3 checkPos = {pos.x, pos.y, from.z};
		// ブロック判定
		if (mapChipField->IsBlocked(checkPos)) {
			return false; // ブロックに遮られた
		}
	}
	return true; // 遮蔽物なし
}

Vector3 Enemy::GetCenterPosition() const {
	const Vector3 offset = {0.0f, 0.0f, 0.0f};
	Vector3 worldPosition = position + offset;
	return worldPosition;
}

void Enemy::OnCollision(Collider* other) {
	
	uint32_t typeID = other->GetTypeID();
	if (typeID == static_cast<uint32_t>(CollisionTypeId::kPlayer)) {
		isHit = true;
	} else if (typeID == static_cast<uint32_t>(CollisionTypeId::kPlayerWeapon)) {
		isHit = true;
	}
}
