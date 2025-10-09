#include "Enemy.h"
#include "Collider/CollisionTypeId.h"
#include "ImGuiManager.h"
#include "TextureManager.h"
#include "Character/Player/Player.h"
#include "LineManager.h"
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

	// --- 追加: 発見記憶ロジック ---
	if (canSeePlayer) {
		lastSeenPlayerPos = player_->GetPosition();
		lastSeenTimer = kLastSeenDuration;
	} else if (lastSeenTimer > 0.0f) {
		lastSeenTimer -= 1.0f / 60.0f; // 60FPS前提
	}

	// 状態遷移
	if (player_) {
		if (canSeePlayer) {
			if (distanceToPlayer > moveStartDistance_) {
				state_ = State::Idle;
			} else if (distanceToPlayer > shootDistance_) {
				state_ = State::Move;
			} else {
				state_ = State::Shoot;
			}
		} else if (lastSeenTimer > 0.0f) {
			state_ = State::Move; // 見失ってもラストスポットへ移動
		} else {
			state_ = State::Idle;
		}
	}

	// プレイヤーの方向を向く（一定速度で回転）
	if (player_ && (state_ == State::Move || state_ == State::Shoot)) {
		Vector3 targetPos = (canSeePlayer) ? player_->GetPosition() : lastSeenPlayerPos;
		Vector3 toTarget = targetPos - position;
		float angleZ = std::atan2(toTarget.y, toTarget.x);
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
	case State::Move: {
		Vector3 targetPos = (canSeePlayer) ? player_->GetPosition() : lastSeenPlayerPos;
		Vector3 dir = targetPos - position;
		dir.z = 0.0f;
		float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
		if (len > 0.1f) {
			dir.x /= len;
			dir.y /= len;
			position.x += dir.x * moveSpeed_;
			position.y += dir.y * moveSpeed_;
		}
		// ラストスポット到達で記憶解除
		if (!canSeePlayer && len < 0.2f) {
			lastSeenTimer = 0.0f;
		}
	} break;
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

void Enemy::OnCollision(Collider* other) {}


void Enemy::Draw() {
	if (object3d) {
		object3d->Draw();
	}
	if (bullet) {
		bullet->Draw();
	}
	DrawViewCone();
	DrawLastSeenMark(); 
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
	Vector3 dirToPlayer = to - from;
	dirToPlayer.z = 0.0f;

	// プレイヤーまでの距離
	float distance = std::sqrt(dirToPlayer.x * dirToPlayer.x + dirToPlayer.y * dirToPlayer.y);
	if (distance > kViewDistance) return false; // 視認距離外

	// 視野角判定
	Vector3 forward = { std::cos(rotation.z), std::sin(rotation.z), 0.0f }; // Z軸回転
	float dot = Vector3::Dot(Vector3::Normalize(forward), Vector3::Normalize(dirToPlayer));
	float angleToPlayer = std::acos(dot) * 180.0f / kPI;
	if (angleToPlayer > kViewAngleDeg / 2.0f) return false; // 視野外

	// ブロック越し判定（従来通り）
	Vector2 start = {from.x, from.y};
	Vector2 end = {to.x, to.y};
	const float step = 0.5f;
	Vector2 dir = end - start;
	float length = std::sqrt(dir.x * dir.x + dir.y * dir.y);
	if (length < 0.001f) return true;
	dir.x /= length; dir.y /= length;
	for (float t = 0.0f; t <= length; t += step) {
		Vector2 pos = start + dir * t;
		Vector3 checkPos = {pos.x, pos.y, from.z};
		if (mapChipField->IsBlocked(checkPos)) {
			return false;
		}
	}
	return true;
}

void Enemy::DrawViewCone() {
	float halfRad = (kViewAngleDeg / 2.0f) * kPI / 180.0f;
	float baseAngle = rotation.z;
	Vector3 center = position;
	// 中心から端への線はそのまま
	for (int i = 0; i < kViewLineDiv; ++i) {
		float a0 = baseAngle - halfRad + (halfRad * 2.0f) * (float(i) / kViewLineDiv);
		Vector3 p0 = center + Vector3{ std::cos(a0) * kViewDistance, std::sin(a0) * kViewDistance, 0.0f };
		LineManager::GetInstance()->DrawLine(center, p0, kViewColor);
	}
	// 外周の弧は分割数-1回だけ描画
	for (int i = 0; i < kViewLineDiv - 1; ++i) {
		float a0 = baseAngle - halfRad + (halfRad * 2.0f) * (float(i) / kViewLineDiv);
		float a1 = baseAngle - halfRad + (halfRad * 2.0f) * (float(i + 1) / kViewLineDiv);
		Vector3 p0 = center + Vector3{ std::cos(a0) * kViewDistance, std::sin(a0) * kViewDistance, 0.0f };
		Vector3 p1 = center + Vector3{ std::cos(a1) * kViewDistance, std::sin(a1) * kViewDistance, 0.0f };
		LineManager::GetInstance()->DrawLine(p0, p1, kViewColor);
	}
}



Vector3 Enemy::GetCenterPosition() const {
	const Vector3 offset = {0.0f, 0.0f, 0.0f};
	Vector3 worldPosition = position + offset;
	return worldPosition;
}

// --- 追加: ラストスポット描画関数 ---
void Enemy::DrawLastSeenMark() {
    if (lastSeenTimer <= 0.0f) return;

    constexpr Vector4 kLastSeenColor = {1.0f, 0.2f, 0.2f, 1.0f}; // 赤色
    constexpr float kLastSeenMarkSize = 0.5f; // マークの大きさ
    const Vector3& center = lastSeenPlayerPos;

    // 十字マーク
    LineManager::GetInstance()->DrawLine(
        center + Vector3{-kLastSeenMarkSize, 0.0f, 0.0f},
        center + Vector3{ kLastSeenMarkSize, 0.0f, 0.0f},
        kLastSeenColor
    );
    LineManager::GetInstance()->DrawLine(
        center + Vector3{0.0f, -kLastSeenMarkSize, 0.0f},
        center + Vector3{0.0f,  kLastSeenMarkSize, 0.0f},
        kLastSeenColor
    );

    // 円（ターゲットマーク外周）
    constexpr int circleDiv = 16;
    for (int i = 0; i < circleDiv; ++i) {
        float a0 = (2.0f * kPI) * (float(i) / circleDiv);
        float a1 = (2.0f * kPI) * (float(i + 1) / circleDiv);
        Vector3 p0 = center + Vector3{ std::cos(a0) * kLastSeenMarkSize, std::sin(a0) * kLastSeenMarkSize, 0.0f };
        Vector3 p1 = center + Vector3{ std::cos(a1) * kLastSeenMarkSize, std::sin(a1) * kLastSeenMarkSize, 0.0f };
        LineManager::GetInstance()->DrawLine(p0, p1, kLastSeenColor);
    }
}
