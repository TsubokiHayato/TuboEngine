#include "PlayerBullet.h"

void PlayerBullet::Initialize(Object3dCommon* object3dCommon, const Vector3& startPos, const Vector3& startVel) {
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
	
	object3d->Draw();
}
