#include "PlayerBullet.h"
#include "Collider/CollisionTypeId.h"
#include "ImGuiManager.h"
#include "Character/Player/Player.h"

// --- 追加: デフォルト値を定数で定義 ---
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
float PlayerBullet::s_disappearRadius = 100.0f;
float PlayerBullet::s_bulletSpeed = 2.0f;
float PlayerBullet::s_disappearZ = 100.0f;
Vector3 PlayerBullet::s_scale = {1.0f, 1.0f, 1.0f};
Vector3 PlayerBullet::s_rotation = {0.0f, 0.0f, 0.0f};
int PlayerBullet::s_damage = 1;
float PlayerBullet::s_fireInterval = 0.2f;

//--------------------------------------------------
// 2点間の距離を計算する関数
//--------------------------------------------------
static float Distance(const Vector3& a, const Vector3& b) {
	float dx = a.x - b.x;
	float dy = a.y - b.y;
	float dz = a.z - b.z;
	return std::sqrt(dx * dx + dy * dy + dz * dz);
}

//--------------------------------------------------
// 初期化処理
//--------------------------------------------------
void PlayerBullet::Initialize( const Vector3& startPos) {
	// 衝突判定のタイプIDを設定
	Collider::SetTypeID(static_cast<uint32_t>(CollisionTypeId::kPlayerWeapon));
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
	// damage = s_damage; // ダメージ設定（未使用）

	// モデルファイル名
	const std::string modelFileNamePath = "star.obj";
	// 3Dオブジェクト生成・初期化
	object3d = std::make_unique<Object3d>();
	object3d->Initialize(modelFileNamePath);
}

//--------------------------------------------------
// 更新処理
//--------------------------------------------------
void PlayerBullet::Update() {
	// 衝突判定の初期化
	isHit = false;
	// パラメータを静的変数から取得
	bulletSpeed = s_bulletSpeed;
	scale = s_scale;
	rotation = s_rotation;

	// プレイヤー向きから進行方向決定
	velocity.x = std::sinf(playerRotation.z) * bulletSpeed;
	velocity.y = -std::cosf(playerRotation.z) * bulletSpeed;
	velocity.z = 0.0f;

	// 今フレームの移動量
	Vector3 desiredMove = velocity; // dt(=1) 前提
	float moveLen2D = std::sqrt(desiredMove.x * desiredMove.x + desiredMove.y * desiredMove.y);

	// ブロック貫通防止用サブステップ
	float tileSize = (mapChipField_) ? MapChipField::GetBlockSize() : 1.0f;
	int subSteps = std::max(1, int(std::ceil(moveLen2D / (tileSize * 0.5f))));
	Vector3 stepMove = desiredMove / float(subSteps);

	for (int i = 0; i < subSteps; ++i) {
		Vector3 nextPos = position + stepMove;
		if (mapChipField_ && mapChipField_->IsBlocked(nextPos)) {
			isAlive = false;
			break;
		}
		position = nextPos;
		if (!isAlive) break;
	}

	// プレイヤーから離れすぎたら消滅
	if (isAlive && Distance(position, playerPostion_) > s_disappearRadius) {
		isAlive = false;
	}

	if (!isAlive) { return; }

	object3d->SetPosition(position);
	object3d->SetRotation(rotation);
	object3d->SetScale(scale);
	object3d->Update();
}

//--------------------------------------------------
// 描画処理
//--------------------------------------------------
void PlayerBullet::Draw() { object3d->Draw(); }

//--------------------------------------------------
// 当たり判定の中心座標を取得
//--------------------------------------------------
Vector3 PlayerBullet::GetCenterPosition() const {
	const Vector3 offset = {0.0f, 0.0f, 0.0f}; // プレイヤーの中心を考慮
	Vector3 worldPosition = position + offset;
	return worldPosition;
}

//--------------------------------------------------
// 衝突時の処理
//--------------------------------------------------
void PlayerBullet::OnCollision(Collider* other) {
	
	// 衝突相手のタイプIDを取得
	uint32_t typeID = other->GetTypeID();
	if (typeID == static_cast<uint32_t>(CollisionTypeId::kEnemy)) {
		// 敵との衝突処理
		isHit = true; // 衝突したらヒットフラグを立てる
		isAlive = false;
	}
}

//--------------------------------------------------
// ImGuiによるグローバルパラメータ調整
//--------------------------------------------------
void PlayerBullet::DrawImGuiGlobal() {

#ifdef USE_IMGUI
	ImGui::Begin("PlayerBullet Parameter (Global)");
	ImGui::SliderFloat("Speed", &s_bulletSpeed, 0.1f, 10.0f, "%.2f");
	ImGui::SliderFloat("Disappear Radius", &s_disappearRadius, 10.0f, 500.0f, "%.1f");
	ImGui::SliderFloat3("Scale", &s_scale.x, 0.1f, 5.0f, "%.2f");
	ImGui::SliderFloat3("Rotation", &s_rotation.x, -3.14f, 3.14f, "%.2f");
	ImGui::SliderInt("Damage", &s_damage, 1, 100);
	ImGui::SliderFloat("Fire Interval", &s_fireInterval, 0.01f, 1.0f, "%.2f");
	ImGui::End();
#endif // USE_IMGUI
}
