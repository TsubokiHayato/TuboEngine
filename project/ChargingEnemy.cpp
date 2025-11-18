#include "ChargingEnemy.h"
#include "Character/Player/Player.h"
#include "Collider/CollisionTypeId.h"
#include "ImGuiManager.h"
#include "Object3d.h"
#include "TextureManager.h"
#include <cmath>

ChargingEnemy::ChargingEnemy() {}
ChargingEnemy::~ChargingEnemy() {}

void ChargingEnemy::Initialize() {
	// 衝突タイプをEnemyと同じにする（基底と同じIDを使う）
	Collider::SetTypeID(static_cast<uint32_t>(CollisionTypeId::kEnemy));

	// 初期トランスフォームは基底 Enemy の SetPosition 等を使う
	SetPosition({0.0f, 0.0f, 5.0f});
	SetRotation({0.0f, 0.0f, 0.0f});
	SetScale({1.0f, 1.0f, 1.0f});

	// 独自の3Dオブジェクト（基底の object3d と重複しない名前）
	object3d_ = std::make_unique<Object3d>();
	object3d_->Initialize("barrier.obj");
	object3d_->SetPosition(GetPosition());
	object3d_->SetRotation(GetRotation());
	object3d_->SetScale(GetScale());

	state_ = State::Idle;
	stateTimer_ = 0.0f;
	isHit_ = false;
}

void ChargingEnemy::Update() {
	// 固定フレーム想定（既存コードが 60fps 前提で動く箇所が多いため）
	const float dt = 1.0f / 60.0f;
	stateTimer_ += dt;

	Player* player = GetPlayerPtr(); // Enemy に追加した protected getter を使う
	Vector3 toPlayer = {0.0f, 0.0f, 0.0f};
	float dist = 1e6f;
	if (player) {
		toPlayer = player->GetPosition() - GetPosition();
		toPlayer.z = 0.0f;
		dist = std::sqrt(toPlayer.x * toPlayer.x + toPlayer.y * toPlayer.y);
	}

	switch (state_) {
	case State::Idle:
		if (player && dist <= detectRadius_) {
			state_ = State::Windup;
			stateTimer_ = 0.0f;
		}
		break;

	case State::Windup:
		// プレイヤーへ向けて回転（テレグラフ）
		if (player) {
			float angle = std::atan2(toPlayer.y, toPlayer.x);
			SetRotation({0.0f, 0.0f, angle});
		}
		if (stateTimer_ >= windupTime_) {
			// 突進開始
			if (player) {
				Vector3 dir = player->GetPosition() - GetPosition();
				dir.z = 0.0f;
				float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
				if (len > 0.0001f) {
					chargeDir_.x = dir.x / len;
					chargeDir_.y = dir.y / len;
				} else {
					float ang = GetRotation().z;
					chargeDir_ = {std::cos(ang), std::sin(ang), 0.0f};
				}
			} else {
				float ang = GetRotation().z;
				chargeDir_ = {std::cos(ang), std::sin(ang), 0.0f};
			}
			state_ = State::Charge;
			stateTimer_ = 0.0f;
			isHit_ = false;
		}
		break;

	case State::Charge:
		// 突進移動
		{
			Vector3 pos = GetPosition();
			pos.x += chargeDir_.x * chargeSpeed_;
			pos.y += chargeDir_.y * chargeSpeed_;
			SetPosition(pos);

			// 回転を移動方向に合わせる
			float ang = std::atan2(chargeDir_.y, chargeDir_.x);
			SetRotation({0.0f, 0.0f, ang});

			if (isHit_ || stateTimer_ >= chargeDuration_) {
				state_ = State::Recover;
				stateTimer_ = 0.0f;
			}
		}
		break;

	case State::Recover:
		if (stateTimer_ >= recoverTime_) {
			state_ = State::Idle;
			stateTimer_ = 0.0f;
			isHit_ = false;
		}
		break;
	}

	// object3d_ を更新（基底の object3d と独立）
	if (object3d_) {
		object3d_->SetPosition(GetPosition());
		object3d_->SetRotation(GetRotation());
		object3d_->SetScale(GetScale());
		if (GetPlayerPtr()) {
			// カメラは基底で設定されることが多いですが、object3d_ にも明示設定しておく
			// object3d_->SetCamera( ... ) は必要に応じて呼んでください
		}
		object3d_->SetCamera(camera_);
		object3d_->Update();
	}
}

void ChargingEnemy::Draw() {
	if (object3d_)
		object3d_->Draw();
}

void ChargingEnemy::DrawImGui() {
	ImGui::Begin("ChargingEnemy");
	ImGui::Text("State: %d", static_cast<int>(state_));
	ImGui::DragFloat("DetectRadius", &detectRadius_, 0.1f, 0.0f, 50.0f);
	ImGui::DragFloat("WindupTime", &windupTime_, 0.01f, 0.0f, 5.0f);
	ImGui::DragFloat("ChargeSpeed", &chargeSpeed_, 0.01f, 0.0f, 5.0f);
	ImGui::DragFloat("ChargeDuration", &chargeDuration_, 0.01f, 0.0f, 5.0f);
	ImGui::DragFloat("RecoverTime", &recoverTime_, 0.01f, 0.0f, 5.0f);
	ImGui::End();
}

void ChargingEnemy::OnCollision(Collider* other) {
	// プレイヤーに当たったらダメージを与える
	if (other->GetTypeID() == static_cast<uint32_t>(CollisionTypeId::kPlayer)) {
		Player* p = GetPlayerPtr();
		if (p) {
			const int damage = 1; // 必要に応じて変更
			int hp = p->GetHP();
			hp -= damage;
			p->SetHP(hp);
			if (hp <= 0) {
				// プレイヤー死亡フラグを立てる
				p->SetIsDead(false);
			}
		}
		// 衝突すると突進は止まる
		isHit_ = true;
		// 追加で発火させたいエフェクトやノックバックがあればここに記述
	}
}

Vector3 ChargingEnemy::GetCenterPosition() const {
	// 基底の GetPosition を利用
	return GetPosition();
}