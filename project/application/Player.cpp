#include "Player.h"
#include "CollisionTypeId.h"
#include "ImGuiManager.h"
#include "Input.h"

//--------------------------------------------------
// コンストラクタ
//--------------------------------------------------
Player::Player() : cooldownTime(0.2f), damageCooldownTimer(0.0f), damageCooldownTime(1.0f) {}

//--------------------------------------------------
// デストラクタ
//--------------------------------------------------
Player::~Player() {}

//--------------------------------------------------
// 初期化処理
//--------------------------------------------------
void Player::Initialize() {

	// プレイヤーのコライダーの設定
	Collider::SetTypeID(static_cast<uint32_t>(CollisionTypeId::kPlayer));

	// プレイヤーの初期位置
	position = Vector3(0.0f, 0.0f, 0.0f);
	// プレイヤーの初期回転
	rotation = Vector3(0.0f, 0.0f, 0.0f);
	// プレイヤーの初期スケール
	scale = Vector3(1.0f, 1.0f, 1.0f);

	// プレイヤーの初期速度
	velocity = Vector3(0.0f, 0.0f, 0.0f);
	// プレイヤーのHP
	HP = 5;
	// プレイヤーの死亡状態
	isAllive = true;

	// モデルファイルパス
	const std::string modelFileNamePath = "sphere.obj";
	// スプライトファイルパス
	const std::string reticleFileNamePath = "2D_Reticle.png";

	// 3Dオブジェクト生成・初期化
	object3d = std::make_unique<Object3d>();
	object3d->Initialize(modelFileNamePath);

	object3d->SetPosition(position);
	object3d->SetRotation(rotation);
	object3d->SetScale(scale);

	// Reticleの初期化
	// Reticleはプレイヤーの中心に配置
	reticleSprite = std::make_unique<Sprite>();
	reticleSprite->Initialize(reticleFileNamePath);

	bulletTimer = 0.0f;
	damageCooldownTimer = 0.0f;
}

//--------------------------------------------------
// 更新処理
//--------------------------------------------------
void Player::Update() {
	isHit = false;
	// ダメージクールダウンタイマー更新
	if (damageCooldownTimer > 0.0f) {
		damageCooldownTimer -= 1.0f / 60.0f;
		if (damageCooldownTimer < 0.0f) damageCooldownTimer = 0.0f;
	}
	Move();
	Rotate();

	

	// 発射タイマー更新
	if (bulletTimer > 0.0f) {
		bulletTimer -= 1.0f / 60.0f; // 60FPS前提
	}

	Shoot();

	// 弾の更新
	for (auto& bullet : bullets) {
		bullet->SetCamera(object3d->GetCamera());
		bullet->Update();
	}

	// isAlive==false のバレットを削除
	bullets.erase(std::remove_if(bullets.begin(), bullets.end(), [](const std::unique_ptr<PlayerBullet>& bullet) { return !bullet->GetIsAlive(); }), bullets.end());

	if (HP <= 0) {
		isAllive = false; // HPが0以下なら死亡状態にする
	}

	object3d->SetPosition(position);
	object3d->SetRotation(rotation);
	object3d->SetScale(scale);
	object3d->Update();

	reticleSprite->SetPosition(reticlePosition);
	reticleSprite->SetGetIsAdjustTextureSize(true); // レティクルのサイズを調整する
	// レティクルの位置を画面中央に設定
	reticleSprite->SetAnchorPoint(Vector2(0.5f, 0.5f)); // アンカーポイントを中央に設定
	reticleSprite->Update();
}

//--------------------------------------------------
// 弾を撃つ処理
//--------------------------------------------------
void Player::Shoot() {
	// スペースキーが押され、発射間隔を満たしていれば弾を生成
	if (Input::GetInstance()->PushKey(DIK_SPACE) && bulletTimer <= 0.0f) {
		auto bullet = std::make_unique<PlayerBullet>();
		bullet->Initialize(position);
		bullet->SetPlayerRotation(rotation);
		bullet->SetPlayerPosition(position);
		bullets.push_back(std::move(bullet));
		bulletTimer = cooldownTime; // クールダウン時間をセット
	}
}

//--------------------------------------------------
// 描画処理
//--------------------------------------------------
void Player::Draw() {
	// 弾の描画
	for (auto& bullet : bullets) {
		bullet->Draw();
	}
	// プレイヤー本体の描画
	object3d->Draw();
}

//--------------------------------------------------
// 移動処理
//--------------------------------------------------
void Player::Move() {
	// 移動前の座標を保存
	Vector3 prevPosition = position;

	// 移動量
	Vector3 moveDelta = {0.0f, 0.0f, 0.0f};
	if (Input::GetInstance()->PushKey(DIK_W)) {
		moveDelta.y -= 0.1f;
	}
	if (Input::GetInstance()->PushKey(DIK_S)) {
		moveDelta.y += 0.1f;
	}
	if (Input::GetInstance()->PushKey(DIK_A)) {
		moveDelta.x -= 0.1f;
	}
	if (Input::GetInstance()->PushKey(DIK_D)) {
		moveDelta.x += 0.1f;
	}

	// 仮移動
	Vector3 tryPosition = position + moveDelta;

	// プレイヤーの大きさ（スケール）を考慮した当たり判定
	if (mapChipField) {
		// プレイヤーの幅・高さ（スケール×ブロックサイズ基準で調整）
		float playerWidth = scale.x * MapChipField::GetBlockWidth()-0.1f;
		float playerHeight = scale.y * MapChipField::GetBlockHeight()-0.1f;

		// 四隅判定（矩形領域がBlockに重なっていないか）
		if (!mapChipField->IsRectBlocked(tryPosition, playerWidth, playerHeight)) {
			// 衝突がなければ位置を更新
			position = tryPosition;
		}
	}
}

///---------------------------------------------------
// 回転処理
//---------------------------------------------------
void Player::Rotate() {
	// --- マウスの方向に身体を向ける処理 ---
	int screenWidth = static_cast<int>(WinApp::GetInstance()->GetClientWidth()); // TODO: DirectXから取得するように
	int screenHeight = static_cast<int>(WinApp::GetInstance()->GetClientHeight());

	int mouseX = static_cast<int>(Input::GetInstance()->GetMousePosition().x);
	int mouseY = static_cast<int>(Input::GetInstance()->GetMousePosition().y);

	float centerX = static_cast<float>(screenWidth) / 2.0f;
	float centerY = static_cast<float>(screenHeight) / 2.0f;

	float dx = static_cast<float>(mouseX) - centerX;
	float dy = static_cast<float>(mouseY) - centerY;

	// Z+前方が0度、X+右方向が+90度
	float angle = std::atan2(dx, -dy);

	rotation.z = angle;
	// Reticleの位置を画面中央に設定
	reticlePosition = Vector2(static_cast<float>(mouseX), static_cast<float>(mouseY));
}

void Player::ReticleDraw() { reticleSprite->Draw(); }

//--------------------------------------------------
// 当たり判定の中心座標を取得
//--------------------------------------------------
Vector3 Player::GetCenterPosition() const {
	const Vector3 offset = {0.0f, 0.0f, 0.0f}; // プレイヤーの中心を考慮
	Vector3 worldPosition = position + offset;
	return worldPosition;
}

//--------------------------------------------------
// 衝突時の処理
//--------------------------------------------------
void Player::OnCollision(Collider* other) {
	// 衝突相手のタイプIDを取得
	uint32_t typeID = other->GetTypeID();
	if (damageCooldownTimer <= 0.0f) {
		if (typeID == static_cast<uint32_t>(CollisionTypeId::kEnemy)) {
			// 敵との衝突処理
			HP -= 1;      // HPを減らす
			isHit = true; // 衝突したらヒットフラグを立てる
			damageCooldownTimer = damageCooldownTime; // ダメージクールダウン開始
		} else if (typeID == static_cast<uint32_t>(CollisionTypeId::kEnemyWeapon)) {
			// 武器との衝突処理
			isHit = true; // 衝突したらヒットフラグを立てる
			damageCooldownTimer = damageCooldownTime; // ダメージクールダウン開始
		}
		// その他の衝突は無敵でも通す
	}
}

//--------------------------------------------------
// ImGuiの描画処理
//--------------------------------------------------
void Player::DrawImGui() {
	position = object3d->GetPosition();
	rotation = object3d->GetRotation();
	scale = object3d->GetScale();

	ImGui::Begin("Player");
	ImGui::Text("HP: %d", HP);
	ImGui::Text("IsHit: %s", isHit ? "Yes" : "No");
	ImGui::Separator();
	ImGui::Text("Cooldown: %.2f / %.2f", bulletTimer, cooldownTime);
	ImGui::Text("%s", (bulletTimer > 0.0f ? "Cooling Down" : "Ready"));
	ImGui::SliderFloat("Cooldown Time", &cooldownTime, 0.05f, 1.0f, "%.2f sec");
	ImGui::Separator();
	ImGui::Text("Damage Cooldown: %.2f / %.2f", damageCooldownTimer, damageCooldownTime);
	ImGui::Text("%s", (damageCooldownTimer > 0.0f ? "Invincible" : "Vulnerable"));
	ImGui::SliderFloat("Damage Cooldown Time", &damageCooldownTime, 0.1f, 3.0f, "%.2f sec");
	// --- 追加: マップチップ種別表示 ---
	if (mapChipField) {
		MapChipField::IndexSet index = mapChipField->GetMapChipIndexSetByPosition(position);
		MapChipType type = mapChipField->GetMapChipTypeByIndex(index.xIndex, index.yIndex);
		const char* typeStr = "Unknown";
		if (type == MapChipType::kBlank)
			typeStr = "Blank";
		else if (type == MapChipType::kBlock)
			typeStr = "Block";
		ImGui::Separator();
		ImGui::Text("MapChip: %s", typeStr);
	}
	ImGui::End();
	object3d->DrawImGui("Player");
	PlayerBullet::DrawImGuiGlobal();
}
