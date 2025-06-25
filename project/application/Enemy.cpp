#include "Enemy.h"
#include"ImGuiManager.h"
#include "Enemy.h"
#include"CollisionTypeId.h"

Enemy::Enemy() {}
Enemy::~Enemy() {}

void Enemy::Initialize(Object3dCommon* object3dCommon) {


	// プレイヤーのコライダーの設定
	Collider::SetTypeID(static_cast<uint32_t>(CollisionTypeId::kEnemy));


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

	// Enemy固有の更新処理

	object3d->SetPosition(position);
	object3d->SetRotation(rotation);
	object3d->SetScale(scale);
	object3d->Update();
}

void Enemy::Draw() {
	
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
	ImGui::Text("Hit: %s", isHit ? "Yes" : "No");
	ImGui::End();
}

void Enemy::Move() {}

Vector3 Enemy::GetCenterPosition() const {
	const Vector3 offset = {0.0f, 0.0f, 0.0f}; // プレイヤーの中心を考慮
	Vector3 worldPosition = position + offset;
	return worldPosition;
}

void Enemy::OnCollision(Collider* other) {
	// 種別IDを取得
	isHit = false; // 衝突判定を初期化

	uint32_t typeID = other->GetTypeID();
	if (typeID == static_cast<uint32_t>(CollisionTypeId::kPlayer)) {
		// 敵との衝突処理
		isHit = true; // 衝突したらヒットフラグを立てる
	} else if (typeID == static_cast<uint32_t>(CollisionTypeId::kWeapon)) {
		// 武器との衝突処理
		isHit = true; // 衝突したらヒットフラグを立てる
	}
}