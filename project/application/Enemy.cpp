#include "Enemy.h"
#include "CollisionTypeId.h"
#include "Enemy.h"
#include "ImGuiManager.h"
#include "TextureManager.h"

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
void Enemy::Update() {
	
	// まず座標・回転・スケールを最新化
	object3d->SetPosition(position);
	object3d->SetRotation(rotation);
	object3d->SetScale(scale);
	object3d->SetCamera(camera_);
	object3d->Update();

	// ヒット演出のトリガー判定
	if (!wasHit && isHit) {
		EmitHitParticle();
	}
	isHit = false;  // 今フレームのヒット状態をリセット
	wasHit = isHit; // 状態を保存

	// Particle
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
	ImGui::End();
}

void Enemy::Move() {}

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
