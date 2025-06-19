#include "Player.h"
#include "ImGuiManager.h"
#include "Input.h"

Player::Player() {}

Player::~Player() {}

void Player::Initialize(Object3dCommon* object3dCommon) {

	object3dCommon_ = object3dCommon;

	// プレイヤーの初期位置
	position = Vector3(0.0f, 0.0f, 0.0f);
	// プレイヤーの初期回転
	rotation = Vector3(0.0f, 0.0f, 0.0f);
	// プレイヤーの初期スケール
	scale = Vector3(1.0f, 1.0f, 1.0f);

	// プレイヤーの初期速度
	velocity = Vector3(0.0f, 0.0f, 0.0f);
	// プレイヤーのHP
	HP = 100;
	// プレイヤーの死亡状態
	isDead = false;

	// モデルファイルパス
	const std::string modelFileNamePath = "barrier.obj";

	object3d = std::make_unique<Object3d>();
	object3d->Initialize(object3dCommon_, modelFileNamePath);

	object3d->SetPosition(position);
	object3d->SetRotation(rotation);
	object3d->SetScale(scale);
}

void Player::Update() {
	isHit = false; // 毎フレームリセット
	Move();
	Shoot();

	// プレイヤーのバレットの移動処理
	for (auto& bullet : bullets) {
		bullet->SetCamera(object3d->GetCamera());
		bullet->Update();
		
	}

	object3d->SetPosition(position);
	object3d->SetRotation(rotation);
	object3d->SetScale(scale);
	object3d->Update();
}

void Player::Shoot() {
	if (Input::GetInstance()->PushKey(DIK_SPACE)) {
		auto bullet = std::make_unique<PlayerBullet>();
		bullet->Initialize(object3dCommon_,position, Vector3(0, 0, 1)); // 前方に発射
		bullets.push_back(std::move(bullet));
	}
}

void Player::Draw() { 
	for (auto& bullet : bullets) {
		bullet->Draw();
	}
	object3d->Draw();

}

void Player::Finalize() {}

void Player::Move() {

	// プレイヤーの移動処理
	if (Input::GetInstance()->PushKey(DIK_W)) {
		position.z += 0.1f;
	}
	if (Input::GetInstance()->PushKey(DIK_S)) {
		position.z -= 0.1f;
	}

	if (Input::GetInstance()->PushKey(DIK_A)) {
		position.x -= 0.1f;
	}
	if (Input::GetInstance()->PushKey(DIK_D)) {
		position.x += 0.1f;
	}
}

void Player::TakeDamage(int damage) {

	HP -= damage;
	if (HP <= 0) {
		isHit = true;
		// ここでプレイヤーの死亡処理を行う
	}

}

void Player::DrawImgui() {

	position = object3d->GetPosition();
	rotation = object3d->GetRotation();
	scale = object3d->GetScale();

	ImGui::Begin("Player");
	ImGui::DragFloat3("Position", &position.x, 0.1f);
	ImGui::DragFloat3("Rotation", &rotation.x, 0.1f);
	ImGui::DragFloat3("Scale", &scale.x, 0.1f);
	ImGui::Text("HP: %d", HP);
	ImGui::Text("IsHit: %s", isHit ? "Yes" : "No");
	ImGui::End();
}
