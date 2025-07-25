#include "EnemyNormalBullet.h"
#include "CollisionTypeId.h"
#include "Enemy.h"
#include "ImGuiManager.h"
#include "Player.h"

// ---  デフォルト値を定数で定義 ---
namespace {
// 弾のデフォルト速度
constexpr float kDefaultBulletSpeed = 2.0f;
// 弾のデフォルトスケール
constexpr Vector3 kDefaultBulletScale = {1.0f, 1.0f, 1.0f};
// 弾のデフォルト回転
constexpr Vector3 kDefaultBulletRotation = {0.0f, 0.0f, 0.0f};
// 弾が消滅するZ座標
constexpr float kBulletDisappearZ = 100.0f;
} // namespace

// 静的メンバ変数の初期化
float EnemyNormalBullet::s_bulletSpeed = kDefaultBulletSpeed;
float EnemyNormalBullet::s_disappearZ = kBulletDisappearZ;
Vector3 EnemyNormalBullet::s_scale = kDefaultBulletScale;
Vector3 EnemyNormalBullet::s_rotation = kDefaultBulletRotation;

//--------------------------------------------------
//	初期化
// --------------------------------------------------
void EnemyNormalBullet::Initialize(const Vector3& startPos) {
	// 衝突判定のタイプIDを設定
	Collider::SetTypeID(static_cast<uint32_t>(CollisionTypeId::kEnemyWeapon));
	// 弾の初期位置を設定
	position = startPos;
	// 速度初期化
	velocity = {};
	// 生存フラグON
	isAlive = true;
	// パラメータを静的変数から取得
	bulletSpeed = s_bulletSpeed;
	disappearZ = s_disappearZ;
	scale = s_scale;
	rotation = s_rotation;
	// 3Dオブジェクト生成・初期化
	object3d = std::make_unique<Object3d>();
	object3d->Initialize("plane.gltf");
}

//--------------------------------------------------
// 更新処理
//--------------------------------------------------
void EnemyNormalBullet::Update() {
	// 衝突判定の初期化
	isHit = false;
	// パラメータを静的変数から取得
	bulletSpeed = s_bulletSpeed;
	scale = s_scale;
	rotation = s_rotation;
	// 弾の進行方向を計算（敵の回転に依存）
	velocity.x = -sinf(enemyRotation_.z) * bulletSpeed;
	velocity.y = -cosf(enemyRotation_.z) * bulletSpeed;
	velocity.z = 0.0f;
	// 位置を更新
	position += velocity;
	// プレイヤーからの距離が一定以上なら消滅
	if (position.z > disappearZ) {
		isAlive = false;
	}
	// 3Dオブジェクトのパラメータを更新
	object3d->SetPosition(position);
	object3d->SetRotation(rotation);
	object3d->SetScale(scale);
	object3d->Update();
}

//--------------------------------------------------
// 描画処理
//--------------------------------------------------
void EnemyNormalBullet::Draw() {
	if (isAlive) {
		object3d->Draw();
	}
}

//--------------------------------------------------
// ImGui描画処理
//--------------------------------------------------
void EnemyNormalBullet::DrawImGuiGlobal() {
	ImGui::Begin("Enemy Normal Bullet");
	ImGui::Text("Enemy Normal Bullet Settings");
	ImGui::DragFloat3("Position", &s_scale.x, 0.1f);
	ImGui::DragFloat3("Rotation", &s_rotation.x, 0.1f);
	ImGui::DragFloat3("Scale", &s_scale.x, 0.1f);
	ImGui::End();
}

//--------------------------------------------------
// 衝突時の処理
//--------------------------------------------------

void EnemyNormalBullet::OnCollision(Collider* other) {
	if (other->GetTypeID() == static_cast<uint32_t>(CollisionTypeId::kPlayer)) {
		isHit = true;
		isAlive = false; // プレイヤーに当たったら弾は消滅
	}
}

//--------------------------------------------------
// 当たり判定の中心座標を取得
//--------------------------------------------------

Vector3 EnemyNormalBullet::GetCenterPosition() const {
	const Vector3 offset = {0.0f, 0.0f, 0.0f}; // 弾の中心を考慮
	Vector3 worldPosition = position + offset;
	return worldPosition;
}


