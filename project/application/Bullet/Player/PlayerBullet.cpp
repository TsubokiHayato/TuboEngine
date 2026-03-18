#include "PlayerBullet.h"
#include "Collider/CollisionTypeId.h"
#include "ImGuiManager.h"
#include "Character/Player/Player.h"

// --- 追加: デフォルト値を定数で定義 ---
namespace {
// 弾のデフォルト速度
constexpr float kDefaultBulletSpeed = 2.0f;
// 弾のデフォルトスケール
constexpr TuboEngine::Math::Vector3 kDefaultBulletScale = {1.0f, 1.0f, 1.0f};
// 弾のデフォルト回転
constexpr TuboEngine::Math::Vector3 kDefaultBulletRotation = {0.0f, 0.0f, 0.0f};
// 弾が消滅するZ座標
constexpr float kBulletDisappearZ = 100.0f;
} // namespace

// 静的メンバ変数の初期化
float PlayerBullet::s_disappearRadius = 100.0f;
float PlayerBullet::s_bulletSpeed = 2.0f;
float PlayerBullet::s_disappearZ = 100.0f;
TuboEngine::Math::Vector3 PlayerBullet::s_scale = {1.0f, 1.0f, 1.0f};
TuboEngine::Math::Vector3 PlayerBullet::s_rotation = {0.0f, 0.0f, 0.0f};
int PlayerBullet::s_damage = 1;
float PlayerBullet::s_fireInterval = 0.2f;

//--------------------------------------------------
// 2点間の距離を計算する関数
//--------------------------------------------------
static float Distance(const TuboEngine::Math::Vector3& a, const TuboEngine::Math::Vector3& b) {
	float dx = a.x - b.x;
	float dy = a.y - b.y;
	float dz = a.z - b.z;
	return std::sqrt(dx * dx + dy * dy + dz * dz);
}

//--------------------------------------------------
// 初期化処理
//--------------------------------------------------
void PlayerBullet::Initialize(const TuboEngine::Math::Vector3& startPos) {
	// 衝突判定のタイプIDを設定
	Collider::SetTypeID(static_cast<uint32_t>(CollisionTypeId::kPlayerWeapon));
	// 弾の初期位置を設定
	position = startPos;

	// 速度初期化 (発射時のプレイヤーの向きを基準にする)
	bulletSpeed = s_bulletSpeed;
	// プレイヤーの回転に基づいた発射方向 (Wii Tank 基準の座標系に合わせる)
	velocity.x = std::sinf(playerRotation.z) * -bulletSpeed;
	velocity.y = -std::cosf(playerRotation.z) * -bulletSpeed;
	velocity.z = 0.0f;

	// 生存フラグON
	isAlive = true;
	reflectCount = 0;
	maxReflectCount = 2; // 2回反射を許可

	// パラメータを静的変数から取得
	disappearZ = s_disappearZ;
	scale = s_scale;
	rotation = s_rotation;

	// モデルファイル名
	const std::string modelFileNamePath = "star.obj";
	// 3Dオブジェクト生成・初期化
	object3d = std::make_unique<TuboEngine::Object3d>();
	object3d->Initialize(modelFileNamePath);
}

//--------------------------------------------------
// 更新処理
//--------------------------------------------------
void PlayerBullet::Update() {
	if (!isAlive) return;

	// 衝突判定の初期化
	isHit = false;
	
	// 更新時のスケール・回転適用
	scale = s_scale;
	rotation = s_rotation;

	// 1フレームの合計移動ベクトル
	TuboEngine::Math::Vector3 moveAmount = velocity; 

	// 衝突判定用のサイズ (見た目に対して適切か確認。2x2のブロックに対して0.5f程度が妥当)
	float bulletWidth = 0.5f;
	float bulletHeight = 0.5f;

	// 高速移動時の突き抜け防止のためサブステップ化
	float moveDist = std::sqrt(moveAmount.x * moveAmount.x + moveAmount.y * moveAmount.y);
	// 0.1f単位でチェックするようにサブステップ数を決定
	int subSteps = std::max(1, int(std::ceil(moveDist / 0.1f))); 
	TuboEngine::Math::Vector3 stepDelta = moveAmount / (float)subSteps;

	for (int i = 0; i < subSteps; ++i) {
		// X軸移動の判定
		TuboEngine::Math::Vector3 nextPosX = position;
		nextPosX.x += stepDelta.x;
		if (mapChipField_ && mapChipField_->IsRectBlocked(nextPosX, bulletWidth, bulletHeight)) {
			if (reflectCount < maxReflectCount) {
				// X反転
				velocity.x *= -1.0f;
				// 今フレームの残りの移動分も反転させる
				stepDelta.x *= -1.0f;
				reflectCount++;
				// 埋まり防止を兼ねて旧位置に留める（反転後の座標を次ステップで使用）
			} else {
				isAlive = false;
				break;
			}
		} else {
			position.x = nextPosX.x;
		}

		// Y軸移動の判定 (更新されたX位置から判定)
		TuboEngine::Math::Vector3 nextPosY = position;
		nextPosY.y += stepDelta.y;
		if (mapChipField_ && mapChipField_->IsRectBlocked(nextPosY, bulletWidth, bulletHeight)) {
			if (reflectCount < maxReflectCount) {
				// Y反転
				velocity.y *= -1.0f;
				stepDelta.y *= -1.0f;
				reflectCount++;
			} else {
				isAlive = false;
				break;
			}
		} else {
			position.y = nextPosY.y;
		}

		if (!isAlive) break;
	}

	// 画面外（Z座標）チェック
	if (position.z > disappearZ) {
		isAlive = false;
	}

	// プレイヤーから離れすぎたら消滅 (反射を考慮して初期位置からの距離で判定するのもありだが、今はプレイヤーからの距離を維持)
	if (isAlive && Distance(position, playerPosition_) > s_disappearRadius) {
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
TuboEngine::Math::Vector3 PlayerBullet::GetCenterPosition() const {
	const TuboEngine::Math::Vector3 offset = {0.0f, 0.0f, 0.0f}; // プレイヤーの中心を考慮
	TuboEngine::Math::Vector3 worldPosition = position + offset;
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
