#include "PlayerBullet.h"
#include"CollisionTypeId.h"

void PlayerBullet::Initialize(Object3dCommon* object3dCommon, const Vector3& startPos, const Vector3& startVel) {
	
	Collider::SetTypeID(static_cast<uint32_t>(CollisionTypeId::kPlayer));

	position = startPos;
	velocity = startVel;
	isAlive = true;

	rotation = Vector3(0.0f, 0.0f, 0.0f);
	scale = Vector3(1.0f, 1.0f, 1.0f);

	// モデルファイルパス
	const std::string modelFileNamePath = "plane.gltf";
	object3d = std::make_unique<Object3d>();
	object3d->Initialize(object3dCommon, modelFileNamePath);
}

void PlayerBullet::Update() {

	position += velocity;
	// 画面外に出たら消滅（例: z座標が100を超えたら消す）
	if (position.z > 100.0f) {
		isAlive = false;
	}

	object3d->SetPosition(position);
	object3d->SetRotation(rotation);
	object3d->SetScale(scale);
	object3d->Update();
}

void PlayerBullet::Draw() {
	
	object3d->Draw(); }


Vector3 PlayerBullet::GetCenterPosition() const {
	const Vector3 offset = {0.0f, 0.0f, 0.0f}; // プレイヤーの中心を考慮
	Vector3 worldPosition = position + offset;
	return worldPosition;
}

void PlayerBullet::OnCollision(Collider* other) {
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