#include "Player.h"
#include "Collider/CollisionTypeId.h"
#include "ImGuiManager.h"
#include "Input.h"
#include "ParticleManager.h"              // 追加: パーティクル生成用
#include "OrbitTrailEmitter.h"            // 追加: 軌道トレイルエミッター
#include "TextureManager.h"
#include "engine/graphic/Particle/ParticleManager.h"
#include "engine/graphic/Particle/RingEmitter.h"

//--------------------------------------------------
// コンストラクタ
//--------------------------------------------------
Player::Player() : cooldownTime(0.2f), damageCooldownTimer(0.0f), damageCooldownTime(1.0f),
	isDodging(false), dodgeTimer(0.0f), dodgeCooldownTimer(0.0f), dodgeDuration(0.2f), dodgeCooldown(1.0f), dodgeSpeed(0.5f), dodgeDirection(0.0f, 0.0f, 0.0f) {}

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
	const std::string modelFileNamePath = "player/Player.obj";
	// スプライトファイルパス
	const std::string reticleFileNamePath = "2D_Reticle.png";

	// 3Dオブジェクト生成・初期化
	object3d = std::make_unique<Object3d>();
	object3d->Initialize(modelFileNamePath);

	object3d->SetPosition(position);
	object3d->SetRotation(rotation);
	object3d->SetScale(scale);

	// Reticleの初期化
	reticleSprite = std::make_unique<Sprite>();
	reticleSprite->Initialize(reticleFileNamePath);

	bulletTimer = 0.0f;
	damageCooldownTimer = 0.0f;
	isDodging = false;
	dodgeTimer = 0.0f;
	dodgeCooldownTimer = 0.0f;
	dodgeDuration = 0.2f;
	dodgeCooldown = 1.0f;
	dodgeSpeed = 0.5f;
	dodgeDirection = Vector3(0.0f, 0.0f, 0.0f);

	// --- 追加: 軌道トレイル用パーティクルエミッター生成 ---
	if (!trailEmitter_) {
		ParticlePreset p{};
		p.name = "PlayerTrail";            // 自動で一意名に調整される可能性あり
		p.texture = "circle2.png";        // 好みで変更
		p.autoEmit = true;                  // 自動発生
		p.emitRate = 60.0f;                 // 毎秒粒子
		p.lifeMin = 0.35f; p.lifeMax = 0.6f;
		p.scaleStart = {0.7f,0.7f,0.7f}; p.scaleEnd = {0.6f,0.6f,0.6f};
		p.colorStart = {0.6f,0.8f,1.0f,0.9f}; p.colorEnd = {0.2f,0.4f,1.0f,0.0f};
		p.maxInstances = 512;               // 移動で多発するので少し多め
		p.billboard = true;
		p.simulateInWorldSpace = true;
		p.center = position;                // 初期中心
		trailEmitter_ = ParticleManager::GetInstance()->CreateEmitter<OrbitTrailEmitter>(p);
		prevPositionTrail_ = position;
	}

	// ダッシュリングエミッタ作成
	if (!dashRingEmitter_) {
		TextureManager::GetInstance()->LoadTexture("gradationLine.png");
		ParticlePreset p{};
		p.name = "PlayerDashRing";
		p.texture = "gradationLine.png";
		p.maxInstances = 16;
		p.autoEmit = false;
		p.burstCount = 1;
		p.lifeMin = 0.35f;
		p.lifeMax = 0.6f;
		p.scaleStart = {0.6f, 0.6f, 1.0f};
		p.scaleEnd   = {1.2f, 1.2f, 1.0f};
		p.colorStart = {0.9f, 0.95f, 1.0f, 0.85f};
		p.colorEnd   = {0.9f, 0.95f, 1.0f, 0.0f};
		p.center = GetPosition();
		// エミッタ中心に追従させる（ワールド空間で独立しない）
		p.simulateInWorldSpace = false;
		dashRingEmitter_ = ParticleManager::GetInstance()->CreateEmitter<RingEmitter>(p);
	}
}

//--------------------------------------------------
// 更新処理
//--------------------------------------------------
void Player::Update() {
	if (isAllive == false) {
		return;
	}// 死亡状態なら更新しない

	if (!isDontMove) {
		isHit = false;
		// ダメージクールダウンタイマー更新
		if (damageCooldownTimer > 0.0f) {
			damageCooldownTimer -= 1.0f / 60.0f;
			if (damageCooldownTimer < 0.0f)
				damageCooldownTimer = 0.0f;
		}
		UpdateDodge();
		// 回避入力（Zキー）
		if (CanDodge() && Input::GetInstance()->PushKey(DIK_SPACE)) {
			StartDodge();
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
	}

	object3d->SetPosition(position);
	object3d->SetRotation(rotation);
	object3d->SetScale(scale);
	object3d->Update();

	reticleSprite->SetPosition(reticlePosition);
	reticleSprite->SetGetIsAdjustTextureSize(true); // レティクルのサイズを調整する
	reticleSprite->SetAnchorPoint(Vector2(0.5f, 0.5f)); // アンカーポイントを中央に設定
	reticleSprite->Update();

	// --- 追加: トレイルエミッター中心更新 (プレイヤー位置) ---
	if (trailEmitter_) {
		trailEmitter_->GetPreset().center = position;
		prevPositionTrail_ = position;
	}
	// 位置追従（カメラ前方オフセット対応）
	if (dashRingEmitter_) {
		Vector3 center = GetPosition();
		if (camera_) {
			Vector3 camRot = camera_->GetRotation();
			// Z回転のみで前方ベクトル（2D平面想定）
			Vector3 forward{ std::cos(camRot.z), std::sin(camRot.z), 0.0f };
			center = center + forward * dashRingOffsetForward_;
		}
		dashRingEmitter_->GetPreset().center = center;
	}

	// 回避開始タイミングでEmit（立ち上がり検出）
	static bool wasDodgingPrevLocal = false; // 関数スコープの前フレーム値
	bool dodgingNow = isDodging;             // 既存の回避フラグを使用
	if (dashRingEmitter_ && dodgingNow && !wasDodgingPrevLocal) {
		TriggerDashRing();
	}
	wasDodgingPrevLocal = dodgingNow;
}

//--------------------------------------------------
// 弾を撃つ処理
//--------------------------------------------------
void Player::Shoot() {
	if (Input::GetInstance()->IsPressMouse(0) && bulletTimer <= 0.0f) {
		auto bullet = std::make_unique<PlayerBullet>();
		bullet->Initialize(position);
		bullet->SetPlayerRotation(rotation);
		bullet->SetPlayerPosition(position);
		bullet->SetMapChipField(mapChipField);
		bullets.push_back(std::move(bullet));
		bulletTimer = cooldownTime;
	}
}

//--------------------------------------------------
// 描画処理
//--------------------------------------------------
void Player::Draw() {
	for (auto& bullet : bullets) { bullet->Draw(); }
	object3d->Draw();
}

//--------------------------------------------------
// 移動処理
//--------------------------------------------------
void Player::Move() {
	if (isDodging) {
		Vector3 tryPosition = position + dodgeDirection * dodgeSpeed;
		if (mapChipField) {
			float playerWidth = scale.x * MapChipField::GetBlockWidth() - 0.1f;
			float playerHeight = scale.y * MapChipField::GetBlockHeight() - 0.1f;
			if (!mapChipField->IsRectBlocked(tryPosition, playerWidth, playerHeight)) {
				position = tryPosition;
			}
		}
		return;
	}
	Vector3 prevPosition = position;
	Vector3 moveDelta = {0.0f, 0.0f, 0.0f};
	if (Input::GetInstance()->PushKey(DIK_W)) { moveDelta.y -= 0.1f; }
	if (Input::GetInstance()->PushKey(DIK_S)) { moveDelta.y += 0.1f; }
	if (Input::GetInstance()->PushKey(DIK_A)) { moveDelta.x -= 0.1f; }
	if (Input::GetInstance()->PushKey(DIK_D)) { moveDelta.x += 0.1f; }
	Vector3 tryPosition = position + moveDelta;
	if (mapChipField) {
		float playerWidth = scale.x * MapChipField::GetBlockWidth()-0.1f;
		float playerHeight = scale.y * MapChipField::GetBlockHeight()-0.1f;
		if (!mapChipField->IsRectBlocked(tryPosition, playerWidth, playerHeight)) {
			position = tryPosition;
		}
	}
}

///---------------------------------------------------
// 回転処理
//---------------------------------------------------
void Player::Rotate() {
	int screenWidth = static_cast<int>(WinApp::GetInstance()->GetClientWidth());
	int screenHeight = static_cast<int>(WinApp::GetInstance()->GetClientHeight());
	int mouseX = static_cast<int>(Input::GetInstance()->GetMousePosition().x);
	int mouseY = static_cast<int>(Input::GetInstance()->GetMousePosition().y);
	float centerX = static_cast<float>(screenWidth) / 2.0f;
	float centerY = static_cast<float>(screenHeight) / 2.0f;
	float dx = static_cast<float>(mouseX) - centerX;
	float dy = static_cast<float>(mouseY) - centerY;
	float angle = std::atan2(dx, -dy);
	rotation.z = angle;
	reticlePosition = Vector2(static_cast<float>(mouseX), static_cast<float>(mouseY));
}

void Player::ReticleDraw() { reticleSprite->Draw(); }

//--------------------------------------------------
// 当たり判定の中心座標を取得
//--------------------------------------------------
Vector3 Player::GetCenterPosition() const {
	const Vector3 offset = {0.0f, 0.0f, 0.0f};
	Vector3 worldPosition = position + offset;
	return worldPosition;
}

//--------------------------------------------------
// 衝突時の処理
//--------------------------------------------------
void Player::OnCollision(Collider* other) {
	if (isDodging) { return; }
	uint32_t typeID = other->GetTypeID();
	if (damageCooldownTimer <= 0.0f) {
		if (typeID == static_cast<uint32_t>(CollisionTypeId::kEnemy)) {
			HP -= 1; isHit = true; damageCooldownTimer = damageCooldownTime;
		} else if (typeID == static_cast<uint32_t>(CollisionTypeId::kEnemyWeapon)) {
			isHit = true; damageCooldownTimer = damageCooldownTime;
		}
	}
	if (other->GetTypeID() == static_cast<uint32_t>(CollisionTypeId::kEnemyWeapon)) {
		HP -= 1; isHit = true;
	}
}

//--------------------------------------------------
// ImGuiの描画処理
//--------------------------------------------------
void Player::DrawImGui() {
#ifdef USE_IMGUI
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
	ImGui::Separator();
	ImGui::Text("Dodge: %s", isDodging ? "Dodging" : (dodgeCooldownTimer > 0.0f ? "Cooldown" : "Ready"));
	ImGui::Text("Dodge Timer: %.2f / %.2f", dodgeTimer, dodgeDuration);
	ImGui::Text("Dodge Cooldown: %.2f / %.2f", dodgeCooldownTimer, dodgeCooldown);
	ImGui::SliderFloat("Dodge Duration", &dodgeDuration, 0.05f, 0.5f, "%.2f sec");
	ImGui::SliderFloat("Dodge Cooldown", &dodgeCooldown, 0.2f, 3.0f, "%.2f sec");
	ImGui::SliderFloat("Dodge Speed", &dodgeSpeed, 0.2f, 2.0f, "%.2f");
	ImGui::Separator();
	ImGui::Text("Dodge Direction: (%.2f, %.2f)", dodgeDirection.x, dodgeDirection.y);
	if (mapChipField) {
		MapChipField::IndexSet index = mapChipField->GetMapChipIndexSetByPosition(position);
		MapChipType type = mapChipField->GetMapChipTypeByIndex(index.xIndex, index.yIndex);
		const char* typeStr = "Unknown";
		if (type == MapChipType::kBlank) typeStr = "Blank"; else if (type == MapChipType::kBlock) typeStr = "Block";
		ImGui::Separator();
		ImGui::Text("MapChip: %s", typeStr);
	}
	// 追加: トレイル調整
	if (trailEmitter_) {
		auto& preset = trailEmitter_->GetPreset();
		ImGui::Separator();
		ImGui::Text("TrailEmitter Instances: %u", preset.maxInstances);
		ImGui::DragFloat3("TrailCenter", &preset.center.x, 0.01f);
		ImGui::DragFloat("TrailEmitRate", &preset.emitRate, 0.1f, 0.0f, 500.0f);
		ImGui::DragFloat2("TrailLifeRange", &preset.lifeMin, 0.01f, 0.05f, 5.0f);
	}
	if (dashRingEmitter_) {
		auto& p = dashRingEmitter_->GetPreset();
		ImGui::Separator();
		ImGui::Text("Dodge Ring");
		ImGui::DragFloat("Ring Offset Forward", &dashRingOffsetForward_, 0.01f, -5.0f, 5.0f);
		ImGui::DragFloat("Ring lifeMin", &p.lifeMin, 0.01f, 0.05f, 5.0f);
		ImGui::DragFloat("Ring lifeMax", &p.lifeMax, 0.01f, 0.05f, 5.0f);
		ImGui::DragFloat3("Ring scaleStart", &p.scaleStart.x, 0.01f, 0.1f, 5.0f);
		ImGui::DragFloat3("Ring scaleEnd", &p.scaleEnd.x, 0.01f, 0.1f, 5.0f);
		ImGui::ColorEdit4("Ring colorStart", &p.colorStart.x);
		ImGui::ColorEdit4("Ring colorEnd", &p.colorEnd.x);
		ImGui::DragInt("Ring burstCount", reinterpret_cast<int*>(&p.burstCount), 1, 1, 16);
		ImGui::Checkbox("Ring autoEmit", &p.autoEmit);
		if (ImGui::Button("Emit Ring")) { dashRingEmitter_->Emit(p.burstCount); }
	}
	ImGui::End();
	object3d->DrawImGui("Player");
	PlayerBullet::DrawImGuiGlobal();
#endif // USE_IMGUI
}

// --- 回避開始 ---
void Player::StartDodge() {
	isDodging = true;
	dodgeTimer = dodgeDuration;
	Vector3 inputDir = GetDodgeInputDirection();
	if (inputDir.x != 0.0f || inputDir.y != 0.0f) {
		float len = std::sqrt(inputDir.x * inputDir.x + inputDir.y * inputDir.y);
		if (len > 0.0f) { inputDir.x /= len; inputDir.y /= len; }
		dodgeDirection = inputDir;
	} else {
		float angle = rotation.z;
		dodgeDirection.x = std::sin(angle);
		dodgeDirection.y = -std::cos(angle);
		dodgeDirection.z = 0.0f;
	}
}

// --- 回避状態更新 ---
void Player::UpdateDodge() {
	if (dodgeCooldownTimer > 0.0f) {
		dodgeCooldownTimer -= 1.0f / 60.0f;
		if (dodgeCooldownTimer < 0.0f) dodgeCooldownTimer = 0.0f;
	}
	if (isDodging) {
		dodgeTimer -= 1.0f / 60.0f;
		if (dodgeTimer <= 0.0f) {
			isDodging = false;
			dodgeCooldownTimer = dodgeCooldown;
		}
	}
}

// --- 回避可能か ---
bool Player::CanDodge() const { return !isDodging && dodgeCooldownTimer <= 0.0f; }

// --- 回避入力方向取得 ---
Vector3 Player::GetDodgeInputDirection() const {
	Vector3 inputDir(0.0f, 0.0f, 0.0f);
	if (Input::GetInstance()->PushKey(DIK_W)) inputDir.y -= 1.0f;
	if (Input::GetInstance()->PushKey(DIK_S)) inputDir.y += 1.0f;
	if (Input::GetInstance()->PushKey(DIK_A)) inputDir.x -= 1.0f;
	if (Input::GetInstance()->PushKey(DIK_D)) inputDir.x += 1.0f;
	return inputDir;
}

// --- ダッシュリングトリガー ---
void Player::TriggerDashRing() {
	if (!dashRingEmitter_) return;
	dashRingEmitter_->Emit(dashRingEmitter_->GetPreset().burstCount);
}
