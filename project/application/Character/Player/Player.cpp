#include "Player.h"
#include "Collider/CollisionTypeId.h"
#include "ImGuiManager.h"
#include "Input.h"
#include"Effects/OrbitTrail/OrbitTrailEmitter.h"
#include "TextureManager.h"
#include "engine/graphic/Particle/ParticleManager.h"
#include "engine/graphic/Particle/Effects/Ring/RingEmitter.h"
#include "engine/graphic/PostEffect/OffScreenRendering.h"
#include "Weapon/PlayerWeapons.h"

//--------------------------------------------------
// コンストラクタ
//--------------------------------------------------
Player::Player()
    : cooldownTime_(0.2f),
      damageCooldownTimer_(0.0f),
      damageCooldownTime_(1.0f),
      isDodging_(false),
      dodgeTimer_(0.0f),
      dodgeCooldownTimer_(0.0f),
      dodgeDuration_(0.2f),
      dodgeCooldown_(1.0f),
      dodgeSpeed_(0.5f),
      dodgeDirection_(0.0f, 0.0f, 0.0f) {}

//--------------------------------------------------
// デストラクタ
//--------------------------------------------------
Player::~Player() {}

//--------------------------------------------------
// 武器パラメータ適用
//--------------------------------------------------
void Player::ApplyWeaponParams(WeaponType type) {
	weaponType_ = type;

	// Strategy 差し替え（弾クラスも武器側で自由に変更できる）
	switch (weaponType_) {
	case WeaponType::Normal:
		weapon_ = PlayerWeapons::CreateNormal();
		break;
	case WeaponType::Rapid:
		weapon_ = PlayerWeapons::CreateRapid();
		break;
	case WeaponType::Shotgun:
		weapon_ = PlayerWeapons::CreateShotgun();
		break;
	default:
		weapon_ = PlayerWeapons::CreateNormal();
		break;
	}

	// 既存のパラメータ互換（ImGui表示/旧処理互換用）
	weaponCooldownSec_ = cooldownTime_;
	weaponBulletSpeed_ = PlayerBullet::s_bulletSpeed;
	weaponPelletCount_ = (weaponType_ == WeaponType::Shotgun) ? 6 : 1;
	weaponSpreadRad_ = (weaponType_ == WeaponType::Shotgun) ? 0.18f : 0.0f;
}

void Player::SetWeaponType(WeaponType type) { ApplyWeaponParams(type); }

void Player::NextWeapon() {
	int t = static_cast<int>(weaponType_);
	t = (t + 1) % 3;
	ApplyWeaponParams(static_cast<WeaponType>(t));
}

void Player::PrevWeapon() {
	int t = static_cast<int>(weaponType_);
	t = (t + 2) % 3;
	ApplyWeaponParams(static_cast<WeaponType>(t));
}

//--------------------------------------------------
// 初期化処理
//--------------------------------------------------
void Player::Initialize() {
	// プレイヤーのコライダーの設定
	Collider::SetTypeID(static_cast<uint32_t>(CollisionTypeId::kPlayer));

	// プレイヤーの初期位置
	position_ = TuboEngine::Math::Vector3(0.0f, 0.0f, 0.0f);
	// プレイヤーの初期回転
	rotation_ = TuboEngine::Math::Vector3(1.56f, 0.0f, 3.12f);
	// プレイヤーの初期スケール
	scale_ = TuboEngine::Math::Vector3(1.0f, 1.0f, 1.0f);

	// プレイヤーの初期速度
	velocity_ = TuboEngine::Math::Vector3(0.0f, 0.0f, 0.0f);
	// プレイヤーのHP
	hp_ = 5;
	// プレイヤーの死亡状態
	isAlive_ = true;

	// モデルファイルパス
	const std::string modelFileNamePath = "player/Player.obj";
	// スプライトファイルパス
	const std::string reticleFileNamePath = "2D_Reticle.png";

	// 3Dオブジェクト生成・初期化
	object3d_ = std::make_unique<Object3d>();
	object3d_->Initialize(modelFileNamePath);

	object3d_->SetPosition(position_);
	object3d_->SetRotation(rotation_);
	object3d_->SetScale(scale_);

	// Reticleの初期化
	reticleSprite_ = std::make_unique<Sprite>();
	reticleSprite_->Initialize(reticleFileNamePath);

	weaponSwitchCooldownTimer_ = 0.0f;
	ApplyWeaponParams(WeaponType::Normal);

	// --- 追加: 軌道トレイル用パーティクルエミッター生成 ---
	if (!trailEmitter_) {
		ParticlePreset p{};
		p.name = "PlayerTrail";    // 自動で一意名に調整される可能性あり
		p.texture = "circle2.png"; // 好みで変更
		p.autoEmit = true;         // 自動発生
		p.emitRate = 60.0f;        // 毎秒粒子
		p.lifeMin = 0.35f;
		p.lifeMax = 0.6f;
		p.scaleStart = {0.7f, 0.7f, 0.7f};
		p.scaleEnd = {0.6f, 0.6f, 0.6f};
		p.colorStart = {0.6f, 0.8f, 1.0f, 0.9f};
		p.colorEnd = {0.2f, 0.4f, 1.0f, 0.0f};
		p.maxInstances = 512; // 移動で多発するので少し多め
		p.billboard = true;
		p.simulateInWorldSpace = true;
		p.center = position_; // 初期中心
		trailEmitter_ = ParticleManager::GetInstance()->CreateEmitter<OrbitTrailEmitter>(p);
		prevPositionTrail_ = position_;
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
		p.scaleEnd = {1.2f, 1.2f, 1.0f};
		p.colorStart = {0.9f, 0.95f, 1.0f, 0.85f};
		p.colorEnd = {0.9f, 0.95f, 1.0f, 0.0f};
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
	if (isAlive_ == false) {
		UpdateWhenDead_();
		return;
	}

	const float dt = 1.0f / 60.0f;
	const bool wantCaptureMouse = ShouldIgnoreMouseForRotate_();

	UpdateGameplayActive_(dt, wantCaptureMouse);

	// 被弾フラグは既存仕様通りこのタイミングで落とす
	isHit_ = false;

	UpdateTransforms_();
	UpdateReticle_();
	UpdateEmitters_();
	UpdateDashRingTrigger_();
	UpdateDashPostEffect_(dt);
	UpdateLowHpVignette_();
}

void Player::UpdateWhenDead_() {
	// 死亡中でも見た目の姿勢は維持（Object3dへ反映）
	object3d_->SetPosition(position_);
	object3d_->SetRotation(rotation_);
	object3d_->SetScale(scale_);
	object3d_->Update();
}

bool Player::ShouldIgnoreMouseForRotate_() const {
#ifdef USE_IMGUI
	return ImGui::GetIO().WantCaptureMouse;
#else
	return false;
#endif
}

void Player::UpdateBullets_(float /*dt*/) {
	for (auto& bullet : bullets_) {
		bullet->SetCamera(object3d_->GetCamera());
		bullet->Update();
	}
	bullets_.erase(std::remove_if(bullets_.begin(), bullets_.end(),
		[](const std::unique_ptr<PlayerBullet>& bullet) { return !bullet->GetIsAlive(); }),
		bullets_.end());
}

void Player::UpdateWeaponSwitch_(float dt) {
	// 武器切替（例: Q/E）
	if (weaponSwitchCooldownTimer_ > 0.0f) {
		weaponSwitchCooldownTimer_ -= dt;
		if (weaponSwitchCooldownTimer_ < 0.0f) {
			weaponSwitchCooldownTimer_ = 0.0f;
		}
	}
	if (weaponSwitchCooldownTimer_ <= 0.0f) {
		if (Input::GetInstance()->TriggerKey(DIK_E)) {
			NextWeapon();
			weaponSwitchCooldownTimer_ = weaponSwitchCooldownSec_;
		} else if (Input::GetInstance()->TriggerKey(DIK_Q)) {
			PrevWeapon();
			weaponSwitchCooldownTimer_ = weaponSwitchCooldownSec_;
		}
	}
}

void Player::UpdateGameplayActive_(float dt, bool wantCaptureMouse) {
	// Clear/Over等の演出シーンでは isMovementLocked=true で入力無効化される。
	if (!wantCaptureMouse && !isMovementLocked_) {
		Rotate();
	}

	if (isMovementLocked_) {
		return;
	}

	// ダメージクールダウンタイマー更新
	if (damageCooldownTimer_ > 0.0f) {
		damageCooldownTimer_ -= dt;
		if (damageCooldownTimer_ < 0.0f) {
			damageCooldownTimer_ = 0.0f;
		}
	}

	UpdateDodge();
	if (CanDodge() && Input::GetInstance()->TriggerKey(DIK_SPACE)) {
		StartDodge();
	}

	Move();

	// Weapon update
	if (weapon_) {
		weapon_->Update(*this, 1.0f / 60.0f);
	}

	// 発射タイマー更新（旧仕様の名残: weapon側へ移行したので維持のみ）
	if (bulletTimer_ > 0.0f) {
		bulletTimer_ -= 1.0f / 60.0f;
	}
	Shoot();
	UpdateBullets_(dt);

	if (hp_ <= 0) {
		isAlive_ = false;
	}

	UpdateWeaponSwitch_(dt);
}

void Player::UpdateTransforms_() {
	object3d_->SetPosition(position_);
	object3d_->SetRotation(rotation_);
	object3d_->SetScale(scale_);
	object3d_->Update();
}

void Player::UpdateReticle_() {
	reticleSprite_->SetPosition(reticlePosition_);
	reticleSprite_->SetGetIsAdjustTextureSize(true);
	reticleSprite_->SetAnchorPoint(TuboEngine::Math::Vector2(0.5f, 0.5f));
	reticleSprite_->Update();
}

void Player::UpdateEmitters_() {
	// --- 追加: トレイルエミッター中心更新 (プレイヤー位置) ---
	if (trailEmitter_) {
		trailEmitter_->GetPreset().center = position_;
		prevPositionTrail_ = position_;
	}
	// 位置追従（カメラ前方オフセット対応）
	if (dashRingEmitter_) {
		TuboEngine::Math::Vector3 center = GetPosition();
		if (camera_) {
			TuboEngine::Math::Vector3 camRot = camera_->GetRotation();
			TuboEngine::Math::Vector3 forward{std::cos(camRot.z), std::sin(camRot.z), 0.0f};
			center = center + forward * dashRingOffsetForward_;
		}
		dashRingEmitter_->GetPreset().center = center;
	}
}

void Player::UpdateDashRingTrigger_() {
	// 回避開始タイミングでEmit（立ち上がり検出）
	static bool wasDodgingPrevLocal = false;
	bool dodgingNow = isDodging_;
	if (dashRingEmitter_ && dodgingNow && !wasDodgingPrevLocal) {
		TriggerDashRing();
		dashPostEffectTimer_ = dashPostEffectDuration_;
		OffScreenRendering::GetInstance()->SetDashPostEffectEnabled(true);
	}
	wasDodgingPrevLocal = dodgingNow;
}

void Player::UpdateDashPostEffect_(float dt) {
	if (dashPostEffectTimer_ > 0.0f) {
		dashPostEffectTimer_ -= dt;
		float t = dashPostEffectTimer_ / std::max(0.0001f, dashPostEffectDuration_);
		t = std::clamp(t, 0.0f, 1.0f);
		float eased = t * t; // ease-out
		OffScreenRendering::GetInstance()->SetDashRadialBlurPower(dashRadialBlurPower_ * eased);
		if (dashPostEffectTimer_ <= 0.0f) {
			dashPostEffectTimer_ = 0.0f;
			OffScreenRendering::GetInstance()->SetDashRadialBlurPower(0.02f);
			OffScreenRendering::GetInstance()->SetDashPostEffectEnabled(false);
		}
	}
}

void Player::UpdateLowHpVignette_() {
	constexpr float kMaxHpAssumed = 5.0f;
	float hpRatio = (kMaxHpAssumed > 0.0f) ? (static_cast<float>(hp_) / kMaxHpAssumed) : 1.0f;
	hpRatio = std::clamp(hpRatio, 0.0f, 1.0f);

	float t = 0.0f;
	if (hpRatio < lowHpVignetteStartRatio_) {
		float denom = std::max(0.0001f, lowHpVignetteStartRatio_);
		t = (lowHpVignetteStartRatio_ - hpRatio) / denom;
	}
	t = std::clamp(t, 0.0f, 1.0f);
	float eased = t * t;
	float targetPower = 0.8f + (lowHpVignetteMaxPower_ - 0.8f) * eased;

	if (lowHpVignetteSmoothing_ <= 0.0f) {
		lowHpVignetteCurrentPower_ = targetPower;
	} else {
		float a = std::clamp(lowHpVignetteSmoothing_, 0.0f, 1.0f);
		lowHpVignetteCurrentPower_ = lowHpVignetteCurrentPower_ + (targetPower - lowHpVignetteCurrentPower_) * a;
	}

	if (lowHpVignetteCurrentPower_ > 0.81f) {
		OffScreenRendering::GetInstance()->SetLowHpVignetteEnabled(true);
		OffScreenRendering::GetInstance()->SetLowHpVignettePower(lowHpVignetteCurrentPower_);
	} else {
		OffScreenRendering::GetInstance()->SetLowHpVignetteEnabled(false);
	}
}

//--------------------------------------------------
// 弾を撃つ処理
//--------------------------------------------------	
void Player::Shoot() {
	// Strategy に委譲
	if (weapon_) {
		weapon_->TryShoot(*this);
	}
}

//--------------------------------------------------
// 描画処理
//--------------------------------------------------	
void Player::Draw() {
	for (auto& bullet : bullets_) {
		bullet->Draw();
	}
	object3d_->Draw();
}

//--------------------------------------------------
// 移動処理
//--------------------------------------------------
void Player::Move() {
	if (isDodging_) {
		TuboEngine::Math::Vector3 tryPosition = position_ + dodgeDirection_ * dodgeSpeed_;
		if (mapChipField_) {
			float playerWidth = scale_.x * MapChipField::GetBlockWidth() - 0.1f;
			float playerHeight = scale_.y * MapChipField::GetBlockHeight() - 0.1f;
			if (!mapChipField_->IsRectBlocked(tryPosition, playerWidth, playerHeight)) {
				position_ = tryPosition;
			}
		}
		return;
	}
	TuboEngine::Math::Vector3 moveDelta = {0.0f, 0.0f, 0.0f};
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
	TuboEngine::Math::Vector3 tryPosition = position_ + moveDelta;
	if (mapChipField_) {
		float playerWidth = scale_.x * MapChipField::GetBlockWidth() - 0.1f;
		float playerHeight = scale_.y * MapChipField::GetBlockHeight() - 0.1f;
		if (!mapChipField_->IsRectBlocked(tryPosition, playerWidth, playerHeight)) {
			position_ = tryPosition;
		}
	}
}


//--------------------------------------------------
// 回転処理
//---------------------------------------------------
void Player::Rotate() {
	// カメラが未設定なら回転を上書きしない（現在のRotationを維持）
	if (!camera_) {
		return;
	}

	int mouseX = static_cast<int>(Input::GetInstance()->GetMousePosition().x);
	int mouseY = static_cast<int>(Input::GetInstance()->GetMousePosition().y);
	reticlePosition_ = TuboEngine::Math::Vector2(static_cast<float>(mouseX), static_cast<float>(mouseY));

	// レイキャストで算出した地面上ターゲット方向で回転を更新（斜め視点対応）
	TuboEngine::Math::Vector3 aimDir = GetAimDirectionFromReticle();
	// 反転補正を削除し、レティクル方向と一致させる
	float angle = std::atan2(aimDir.x, -aimDir.y);
	rotation_.z = 3.12f+angle;
}


void Player::ReticleDraw() { reticleSprite_->Draw(); }

//--------------------------------------------------
// 当たり判定の中心座標を取得
//--------------------------------------------------
TuboEngine::Math::Vector3 Player::GetCenterPosition() const {
	const TuboEngine::Math::Vector3 offset = {0.0f, 0.0f, 0.0f};
	TuboEngine::Math::Vector3 worldPosition = position_ + offset;
	return worldPosition;
}

//--------------------------------------------------
// 衝突時の処理
//--------------------------------------------------
void Player::OnCollision(Collider* other) {
	if (isDodging_) {
		return;
	}
	uint32_t typeID = other->GetTypeID();
	if (damageCooldownTimer_ <= 0.0f) {
		if (typeID == static_cast<uint32_t>(CollisionTypeId::kEnemy)) {
			hp_ -= 1;
			isHit_ = true;
			damageCooldownTimer_ = damageCooldownTime_;
		} else if (typeID == static_cast<uint32_t>(CollisionTypeId::kEnemyWeapon)) {
			isHit_ = true;
			damageCooldownTimer_ = damageCooldownTime_;
		}
	}
	if (other->GetTypeID() == static_cast<uint32_t>(CollisionTypeId::kEnemyWeapon)) {
		hp_ -= 1;
		isHit_ = true;
	}
}

//--------------------------------------------------
// ImGuiの描画処理
//--------------------------------------------------
void Player::DrawImGui() {
#ifdef USE_IMGUI
	position_ = object3d_->GetPosition();
	rotation_ = object3d_->GetRotation();
	scale_ = object3d_->GetScale();
	ImGui::Begin("Player");
	ImGui::Text("HP: %d", hp_);
	ImGui::Text("IsHit: %s", isHit_ ? "Yes" : "No");
	ImGui::Separator();
	ImGui::Text("Cooldown: %.2f / %.2f", bulletTimer_, cooldownTime_);
	ImGui::Text("%s", (bulletTimer_ > 0.0f ? "Cooling Down" : "Ready"));
	ImGui::SliderFloat("Cooldown Time", &cooldownTime_, 0.05f, 1.0f, "%.2f sec");
	ImGui::Separator();
	ImGui::Text("Damage Cooldown: %.2f / %.2f", damageCooldownTimer_, damageCooldownTime_);
	ImGui::Text("%s", (damageCooldownTimer_ > 0.0f ? "Invincible" : "Vulnerable"));
	ImGui::SliderFloat("Damage Cooldown Time", &damageCooldownTime_, 0.1f, 3.0f, "%.2f sec");
	ImGui::Separator();
	ImGui::Text("Dodge: %s", isDodging_ ? "Dodging" : (dodgeCooldownTimer_ > 0.0f ? "Cooldown" : "Ready"));
	ImGui::Text("Dodge Timer: %.2f / %.2f", dodgeTimer_, dodgeDuration_);
	ImGui::Text("Dodge Cooldown: %.2f / %.2f", dodgeCooldownTimer_, dodgeCooldown_);
	ImGui::SliderFloat("Dodge Duration", &dodgeDuration_, 0.05f, 0.5f, "%.2f sec");
	ImGui::SliderFloat("Dodge Cooldown", &dodgeCooldown_, 0.2f, 3.0f, "%.2f sec");
	ImGui::SliderFloat("Dodge Speed", &dodgeSpeed_, 0.2f, 2.0f, "%.2f");
	ImGui::Separator();
	ImGui::Text("Dodge Direction: (%.2f, %.2f)", dodgeDirection_.x, dodgeDirection_.y);
	if (mapChipField_) {
		MapChipField::IndexSet index = mapChipField_->GetMapChipIndexSetByPosition(position_);
		MapChipType type = mapChipField_->GetMapChipTypeByIndex(index.xIndex, index.yIndex);
		const char* typeStr = "Unknown";
		if (type == MapChipType::kBlank)
			typeStr = "Blank";
		else if (type == MapChipType::kBlock)
			typeStr = "Block";
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
		if (ImGui::Button("Emit Ring")) {
			dashRingEmitter_->Emit(p.burstCount);
		}
	}
	const char* weaponName = weapon_ ? weapon_->GetName() : "(null)";
	ImGui::Separator();
	ImGui::Text("Weapon(Strategy): %s", weaponName);
	if (ImGui::Button("Next Weapon")) { NextWeapon(); }
	ImGui::SameLine();
	if (ImGui::Button("Prev Weapon")) { PrevWeapon(); }
	ImGui::End();
	object3d_->DrawImGui("Player");
	PlayerBullet::DrawImGuiGlobal();
#endif // USE_IMGUI
}

// --- 回避開始 ---
void Player::StartDodge() {
	isDodging_ = true;
	dodgeTimer_ = dodgeDuration_;
	TuboEngine::Math::Vector3 inputDir = GetDodgeInputDirection();
	if (inputDir.x != 0.0f || inputDir.y != 0.0f) {
		float len = std::sqrt(inputDir.x * inputDir.x + inputDir.y * inputDir.y);
		if (len > 0.0f) {
			inputDir.x /= len;
			inputDir.y /= len;
		}
		dodgeDirection_ = inputDir;
	} else {
		float angle = rotation_.z;
		dodgeDirection_.x = std::sin(angle);
		dodgeDirection_.y = -std::cos(angle);
		dodgeDirection_.z = 0.0f;
	}
}

// --- 回避状態更新 ---
void Player::UpdateDodge() {
	if (dodgeCooldownTimer_ > 0.0f) {
		dodgeCooldownTimer_ -= 1.0f / 60.0f;
		if (dodgeCooldownTimer_ < 0.0f)
			dodgeCooldownTimer_ = 0.0f;
	}
	if (isDodging_) {
		dodgeTimer_ -= 1.0f / 60.0f;
		if (dodgeTimer_ <= 0.0f) {
			isDodging_ = false;
			dodgeCooldownTimer_ = dodgeCooldown_;
		}
	}
}

// --- 回避可能か ---
bool Player::CanDodge() const { return !isDodging_ && dodgeCooldownTimer_ <= 0.0f; }

// --- 回避入力方向取得 ---
TuboEngine::Math::Vector3 Player::GetDodgeInputDirection() const {
	TuboEngine::Math::Vector3 inputDir(0.0f, 0.0f, 0.0f);
	if (Input::GetInstance()->PushKey(DIK_W))
		inputDir.y -= 1.0f;
	if (Input::GetInstance()->PushKey(DIK_S))
		inputDir.y += 1.0f;
	if (Input::GetInstance()->PushKey(DIK_A))
		inputDir.x -= 1.0f;
	if (Input::GetInstance()->PushKey(DIK_D))
		inputDir.x += 1.0f;
	return inputDir;
}

// --- ダッシュリングトリガー ---
void Player::TriggerDashRing() {
	if (!dashRingEmitter_)
		return;
	dashRingEmitter_->Emit(dashRingEmitter_->GetPreset().burstCount);
}

// --- レティクルから地面へのレイキャストでエイム方向取得（斜め視点対応） ---
TuboEngine::Math::Vector3 Player::GetAimDirectionFromReticle() const {
	TuboEngine::Math::Vector3 dir{0.0f, -1.0f, 0.0f};
	if (!camera_) {
		return dir; // カメラ未設定なら従来の前方
	}
	// スクリーン座標からNDCに変換
	float screenW = static_cast<float>(TuboEngine::WinApp::GetInstance()->GetClientWidth());
	float screenH = static_cast<float>(TuboEngine::WinApp::GetInstance()->GetClientHeight());
	TuboEngine::Math::Vector2 mouse = Input::GetInstance()->GetMousePosition();
	float ndcX = (mouse.x / screenW) * 2.0f - 1.0f;
	float ndcY = 1.0f - (mouse.y / screenH) * 2.0f; // 上が+1

	// カメラのViewProjection逆行列を計算（DirectXMath）
	const Matrix4x4& vp = camera_->GetViewProjectionMatrix();
	DirectX::XMMATRIX xmVP = DirectX::XMMatrixSet(
		vp.m[0][0], vp.m[0][1], vp.m[0][2], vp.m[0][3],
		vp.m[1][0], vp.m[1][1], vp.m[1][2], vp.m[1][3],
		vp.m[2][0], vp.m[2][1], vp.m[2][2], vp.m[2][3],
		vp.m[3][0], vp.m[3][1], vp.m[3][2], vp.m[3][3]
	);
	DirectX::XMVECTOR det;
	DirectX::XMMATRIX xmInvVP = DirectX::XMMatrixInverse(&det, xmVP);
	// ヘルパー: アンプロジェクト
	auto unproject = [&](float x, float y, float z) {
		DirectX::XMVECTOR p = DirectX::XMVectorSet(x, y, z, 1.0f);
		DirectX::XMVECTOR w = DirectX::XMVector4Transform(p, xmInvVP);
		DirectX::XMFLOAT4 wf;
		DirectX::XMStoreFloat4(&wf, w);
		if (std::fabs(wf.w) > 1e-6f) {
			wf.x /= wf.w; wf.y /= wf.w; wf.z /= wf.w;
		}
		return TuboEngine::Math::Vector3{wf.x, wf.y, wf.z};
	};
	TuboEngine::Math::Vector3 worldNear = unproject(ndcX, ndcY, 0.0f);
	TuboEngine::Math::Vector3 worldFar = unproject(ndcX, ndcY, 1.0f);
	TuboEngine::Math::Vector3 rayOrigin = worldNear;
	TuboEngine::Math::Vector3 rayDir = {worldFar.x - worldNear.x, worldFar.y - worldNear.y, worldFar.z - worldNear.z};
	float len = std::sqrt(rayDir.x * rayDir.x + rayDir.y * rayDir.y + rayDir.z * rayDir.z);
	if (len > 0.0f) { rayDir.x /= len; rayDir.y /= len; rayDir.z /= len; }
	// Z=0 平面と交差（地面）
	if (std::fabs(rayDir.z) < 1e-5f) { return dir; }
	float t = (0.0f - rayOrigin.z) / rayDir.z;
	TuboEngine::Math::Vector3 hit = {rayOrigin.x + rayDir.x * t, rayOrigin.y + rayDir.y * t, 0.0f};
	TuboEngine::Math::Vector3 aim = {hit.x - position_.x, hit.y - position_.y, hit.z - position_.z};
	float ilen = std::sqrt(aim.x * aim.x + aim.y * aim.y + aim.z * aim.z);
	if (ilen > 0.0f) { aim.x /= ilen; aim.y /= ilen; aim.z /= ilen; }
	return aim;
}
