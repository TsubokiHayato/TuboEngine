#include "Enemy.h"
#include"ImGuiManager.h"

Enemy::Enemy() {}
Enemy::~Enemy() {}

void Enemy::Initialize(Object3dCommon* object3dCommon) {
	BaseCharacter::Initialize(object3dCommon);
	// Enemy固有の初期化
	position = {0.0f,0.0f,5.0f};
	rotation = {};
	scale = {1.0f, 1.0f, 1.0f}; // 初期スケール


	object3d = std::make_unique<Object3d>();
	const std::string modelFileNamePath = "barrier.obj"; // 適切なモデルファイルパスを指定
	object3d->Initialize(object3dCommon, modelFileNamePath);
	object3d->SetPosition(position);
	object3d->SetRotation(rotation);
	object3d->SetScale(scale);
}

void Enemy::Update() {
	BaseCharacter::Update();
	// Enemy固有の更新処理

	object3d->SetPosition(position);
	object3d->SetRotation(rotation);
	object3d->SetScale(scale);
	object3d->Update();
}

void Enemy::Draw() {
	BaseCharacter::Draw();
	// Enemy固有の描画処理
	if (object3d) {
		object3d->Draw();
	}
}

void Enemy::DrawImGui() { 
	ImGui::Begin("Enemy");
	ImGui::Text("Enemy Position: (%.2f, %.2f, %.2f)", position.x, position.y, position.z);
	ImGui::Text("Enemy Velocity: (%.2f, %.2f, %.2f)", velocity.x, velocity.y, velocity.z);
	ImGui::Text("Enemy HP: %d", HP);
	ImGui::Text("Enemy Alive: %s", isAlive ? "Yes" : "No");
	ImGui::End();
}

void Enemy::Move() {
	BaseCharacter::Move();
	// Enemy固有の移動処理
}

void Enemy::TakeDamage(int damage) {
	BaseCharacter::TakeDamage(damage);
	// Enemy固有のダメージ処理
}
