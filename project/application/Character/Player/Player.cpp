#include "Player.h"
#include "PlayerAutoController.h" // 追加
#include "Collider/CollisionTypeId.h"
#include "ImGuiManager.h"
#include "Input.h"
#include "Effects/OrbitTrail/OrbitTrailEmitter.h"
#include "TextureManager.h"
#include "engine/graphic/Particle/ParticleManager.h"
#include "engine/graphic/Particle/Effects/Ring/RingEmitter.h"
#include "engine/graphic/PostEffect/OffScreenRendering.h"

//--------------------------------------------------
// コンストラクタ
//--------------------------------------------------
Player::Player()
    : cooldownTime(0.2f), damageCooldownTimer(0.0f), damageCooldownTime(1.0f), isDodging(false), dodgeTimer(0.0f), dodgeCooldownTimer(0.0f), dodgeDuration(0.2f), dodgeCooldown(1.0f), dodgeSpeed(0.5f),
      dodgeDirection(0.0f, 0.0f, 0.0f) {
    autoController_.Initialize(this);
}

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
	position = TuboEngine::Math::Vector3(0.0f, 0.0f, 0.0f);
	// プレイヤーの初期回転
	rotation = TuboEngine::Math::Vector3(1.56f, 0.0f, 3.12f);
	// プレイヤーの初期スケール
	scale = TuboEngine::Math::Vector3(1.0f, 1.0f, 1.0f);

	// プレイヤーの初期速度
	velocity = TuboEngine::Math::Vector3(0.0f, 0.0f, 0.0f);
	// プレイヤーのHP
	HP = 5;
	// プレイヤーの死亡状態
	isAlive = true;

	// モデルファイルパス
	const std::string modelFileNamePath = "player/Player.obj";
	// スプライトファイルパス
	const std::string reticleFileNamePath = "2D_Reticle.png";

	// 3Dオブジェクト生成・初期化
	object3d = std::make_unique<TuboEngine::Object3d>();
	object3d->Initialize(modelFileNamePath);

	object3d->SetPosition(position);
	object3d->SetRotation(rotation);
	object3d->SetScale(scale);

	// Reticleの初期化
	reticleSprite = std::make_unique<TuboEngine::Sprite>();
	reticleSprite->Initialize(reticleFileNamePath);

	bulletTimer = 0.0f;
	damageCooldownTimer = 0.0f;
	isDodging = false;
	dodgeTimer = 0.0f;
	dodgeCooldownTimer = 0.0f;
	dodgeDuration = 0.2f;
	dodgeCooldown = 1.0f;
	dodgeSpeed = 0.5f;
	dodgeDirection = TuboEngine::Math::Vector3(0.0f, 0.0f, 0.0f);

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
		p.center = position; // 初期中心
		trailEmitter_ = TuboEngine::ParticleManager::GetInstance()->CreateEmitter<OrbitTrailEmitter>(p);
		prevPositionTrail_ = position;
	}

	// ダッシュリングエミッタ作成
	if (!dashRingEmitter_) {
		TuboEngine::TextureManager::GetInstance()->LoadTexture("gradationLine.png");
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
		dashRingEmitter_ = TuboEngine::ParticleManager::GetInstance()->CreateEmitter<RingEmitter>(p);
	}
}

//--------------------------------------------------
// 更新処理
//--------------------------------------------------
void Player::Update() {
	if (isAlive == false) {
		// 死亡中でも見た目の姿勢は維持（Object3dへ反映）
		object3d->SetPosition(position);
		object3d->SetRotation(rotation);
		object3d->SetScale(scale);
     SetModelAlpha(1.0f);
		object3d->Update();
		return;
	} // 死亡状態なら更新しない

#ifdef USE_IMGUI
	// ImGuiの操作中はゲーム側の自動回転（マウス追従）で上書きしない
	const bool wantCaptureMouse = ImGui::GetIO().WantCaptureMouse;
#else
	const bool wantCaptureMouse = false;
#endif

	// デモ用自動操作フラグがONのときは AI で移動・射撃を制御する
    autoController_.Update(1.0f / 60.0f);

	// 回転処理
	if (!isMovementLocked) {
		if (IsAutoControlEnabled()) {
			// オート操作中: autoAimDir_ から回転を計算
			TuboEngine::Math::Vector3 dir = autoAimDir_;
			float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
			if (len > 0.0001f) {
				dir.x /= len;
				dir.y /= len;
				// Rotate() と同じ式: atan2(aimDir.x, -aimDir.y)
				float angle = std::atan2(dir.x, -dir.y);
				rotation.z = 3.12f + angle;
			}
		} else if (!wantCaptureMouse) {
			// 手動時は従来通りマウス方向
			Rotate();
		}
	}

	if (!isMovementLocked) {
		// ※isHit は OnCollision で立つ。ここで毎フレーム落とすと「被弾したフレーム」を取り逃すので
		// 演出検出後に落とす。
		// ダメージクールダウンタイマー更新
		if (damageCooldownTimer > 0.0f) {
			damageCooldownTimer -= 1.0f / 60.0f;
			if (damageCooldownTimer < 0.0f)
				damageCooldownTimer = 0.0f;
		}
		UpdateDodge();
		// 回避入力（SPACEキー）
		// 長押し(PushKey)だと、クールダウン明けに押しっぱなしで即ダッシュしてしまうので
		// 押した瞬間(TriggerKey)でのみ開始する。
		if (CanDodge() && TuboEngine::Input::GetInstance()->TriggerKey(DIK_SPACE)) {
			StartDodge();
		}
		Move();
		// Rotate() は上で一度だけ行う（ImGui操作中はスキップ）
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
			isAlive = false; // HPが0以下なら死亡状態にする
		}
	}

	// 被弾フラグは既存仕様通りこのタイミングで落とす
	isHit = false;

 // 被弾クールダウン中は点滅（アルファ変化）
	if (damageCooldownTimer > 0.0f) {
       damageBlinkTime_ += 1.0f / 60.0f;
		// 0.08秒周期で点滅（分かりやすめ）
		constexpr float kBlinkPeriod = 0.08f;
		float phase = std::fmod(damageBlinkTime_, kBlinkPeriod) / kBlinkPeriod;
		float alpha = (phase < 0.5f) ? 0.15f : 1.0f;
		SetModelAlpha(alpha);
	} else {
        damageBlinkTime_ = 0.0f;
		SetModelAlpha(1.0f);
	}

	object3d->SetPosition(position);
	object3d->SetRotation(rotation);
	object3d->SetScale(scale);
	object3d->Update();

	reticleSprite->SetPosition(reticlePosition);
	reticleSprite->SetGetIsAdjustTextureSize(true);     // レティクルのサイズを調整する
	reticleSprite->SetAnchorPoint(TuboEngine::Math::Vector2(0.5f, 0.5f)); // アンカーポイントを中央に設定
	reticleSprite->Update();

	// --- 追加: トレイルエミッター中心更新 (プレイヤー位置) ---
	if (trailEmitter_) {
		trailEmitter_->GetPreset().center = position;
		prevPositionTrail_ = position;
	}
	// 位置追従（カメラ前方オフセット対応）
	if (dashRingEmitter_) {
		TuboEngine::Math::Vector3 center = GetPosition();
		if (camera_) {
			TuboEngine::Math::Vector3 camRot = camera_->GetRotation();
			// Z回転のみで前方ベクトル（2D平面想定）
			TuboEngine::Math::Vector3 forward{std::cos(camRot.z), std::sin(camRot.z), 0.0f};
			center = center + forward * dashRingOffsetForward_;
		}
		dashRingEmitter_->GetPreset().center = center;
	}

	// 回避開始タイミングでEmit（立ち上がり検出）
	static bool wasDodgingPrevLocal = false; // 関数スコープの前フレーム値
	bool dodgingNow = isDodging;             // 既存の回避フラグを使用
	if (dashRingEmitter_ && dodgingNow && !wasDodgingPrevLocal) {
		TriggerDashRing();
		// Dash演出: ポストエフェクトを一時的にRadialBlurへ
		dashPostEffectTimer_ = dashPostEffectDuration_;
		OffScreenRendering::GetInstance()->SetDashPostEffectEnabled(true);
	}
	wasDodgingPrevLocal = dodgingNow;

	// Dashポストエフェクトの時間経過で自動復帰
	if (dashPostEffectTimer_ > 0.0f) {
		dashPostEffectTimer_ -= 1.0f / 60.0f;
		// 0→1 の進行度（開始直後=1、終了直前=0）
		float t = dashPostEffectTimer_ / std::max(0.0001f, dashPostEffectDuration_);
		t = std::clamp(t, 0.0f, 1.0f);
		// 立ち上がりで強く、徐々に弱まる（イージング）
		float eased = t * t; // ease-out
		OffScreenRendering::GetInstance()->SetDashRadialBlurPower(dashRadialBlurPower_ * eased);
		if (dashPostEffectTimer_ <= 0.0f) {
			dashPostEffectTimer_ = 0.0f;
			OffScreenRendering::GetInstance()->SetDashRadialBlurPower(0.02f); // RadialBlurのデフォルトへ戻す
			OffScreenRendering::GetInstance()->SetDashPostEffectEnabled(false);
		}
	}

	// 履歴追加
	positionHistory_.push_back(GetCenterPosition());
	const size_t maxHistoryCount = 180;
	if (positionHistory_.size() > maxHistoryCount) {
		positionHistory_.pop_front();
	}
}

//--------------------------------------------------
// 見た目だけ更新（ゲームロジックなし。Transition用）
//--------------------------------------------------
void Player::UpdateVisualOnly() {
	object3d->SetPosition(position);
	object3d->SetRotation(rotation);
	object3d->SetScale(scale);
	object3d->Update();

	// Transition 等で Update() を回さない場合でも、移動に追従した演出を出す
	if (trailEmitter_) {
		trailEmitter_->GetPreset().center = position;
	}
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

TuboEngine::Math::Vector3 Player::GetPastCenterPosition(int delayFrames) const {
	if (positionHistory_.empty()) {
		return GetCenterPosition(); // 履歴がない場合は現在位置
	}
	if (delayFrames < 0) {
		delayFrames = 0;
	}
	int index = static_cast<int>(positionHistory_.size()) - 1 - delayFrames;
	if (index < 0) {
		index = 0; // 足りない場合は一番古いものを返す
	}
	return positionHistory_[index];
}

//--------------------------------------------------
// 弾を撃つ処理
//--------------------------------------------------	
void Player::Shoot() {
	bool trigger = false;
	if (IsAutoControlEnabled()) {
		trigger = autoShoot_;
	} else {
		trigger = TuboEngine::Input::GetInstance()->IsPressMouse(0);
	}

	if (trigger && bulletTimer <= 0.0f) {
		// 発射
		auto bullet = std::make_unique<PlayerBullet>();
		bullet->SetPlayerRotation(rotation);
		bullet->SetPlayerPosition(position);
		bullet->SetMapChipField(mapChipField);
		
		// フィールドや回転を設定した後に初期化（Initialize内で速度が決まる）
		bullet->Initialize(position);
		
		bullets.push_back(std::move(bullet));
		bulletTimer = cooldownTime;
	}
}

//--------------------------------------------------
// 弾を全消去する処理
//--------------------------------------------------
void Player::ClearBullets() {
	bullets.clear();
}

//--------------------------------------------------
// 描画処理
//--------------------------------------------------	
void Player::Draw() {
	for (auto& bullet : bullets) {
		bullet->Draw();
	}
	object3d->Draw();
}

//--------------------------------------------------
// 移動処理
//--------------------------------------------------
void Player::Move() {
	if (isDodging) {
		TuboEngine::Math::Vector3 tryPosition = position + dodgeDirection * dodgeSpeed;
		if (mapChipField) {
			float playerWidth = scale.x * MapChipField::GetBlockWidth() - 0.1f;
			float playerHeight = scale.y * MapChipField::GetBlockHeight() - 0.1f;
			if (!mapChipField->IsRectBlocked(tryPosition, playerWidth, playerHeight)) {
				position = tryPosition;
			}
		}
		// 回避中は速度を更新しておく（回避終了後の慣性のために）
		velocity = dodgeDirection * dodgeSpeed;
		return;
	}

	TuboEngine::Math::Vector3 moveInput = {0.0f, 0.0f, 0.0f};
	// 入力取得
	if (IsAutoControlEnabled()) {
		moveInput = autoMoveDir_;
	} else {
		if (TuboEngine::Input::GetInstance()->PushKey(DIK_W)) moveInput.y -= 1.0f;
		if (TuboEngine::Input::GetInstance()->PushKey(DIK_S)) moveInput.y += 1.0f;
		if (TuboEngine::Input::GetInstance()->PushKey(DIK_A)) moveInput.x -= 1.0f;
		if (TuboEngine::Input::GetInstance()->PushKey(DIK_D)) moveInput.x += 1.0f;
	}

	// 入力の正規化
	float inputLen = std::sqrt(moveInput.x * moveInput.x + moveInput.y * moveInput.y);
	if (inputLen > 0.0f) {
		moveInput.x /= inputLen;
		moveInput.y /= inputLen;
	}

	// 戦車物理パラメータ
	float acceleration = 0.015f; // 加速度
	float friction = 0.92f;      // 摩擦（減衰率）
	float maxSpeed = 0.15f;      // 最大速度

	// 加速
	velocity.x += moveInput.x * acceleration;
	velocity.y += moveInput.y * acceleration;

	// 摩擦（入力がない時にゆっくり止まる）
	if (inputLen == 0.0f) {
		velocity.x *= friction;
		velocity.y *= friction;
	}

	// 速度制限
	float speed = std::sqrt(velocity.x * velocity.x + velocity.y * velocity.y);
	if (speed > maxSpeed) {
		velocity.x = (velocity.x / speed) * maxSpeed;
		velocity.y = (velocity.y / speed) * maxSpeed;
	}

	// 衝突判定付きの位置更新
	TuboEngine::Math::Vector3 tryPosition = position + velocity;
	if (mapChipField) {
		float playerWidth = scale.x * MapChipField::GetBlockWidth() - 0.1f;
		float playerHeight = scale.y * MapChipField::GetBlockHeight() - 0.1f;
		
		// X方向の移動チェック
		TuboEngine::Math::Vector3 tryX = position;
		tryX.x += velocity.x;
		if (!mapChipField->IsRectBlocked(tryX, playerWidth, playerHeight)) {
			position.x = tryX.x;
		} else {
			velocity.x = 0.0f; // 壁に当たったら速度を殺す
		}

		// Y方向の移動チェック
		TuboEngine::Math::Vector3 tryY = position;
		tryY.y += velocity.y;
		if (!mapChipField->IsRectBlocked(tryY, playerWidth, playerHeight)) {
			position.y = tryY.y;
		} else {
			velocity.y = 0.0f; // 壁に当たったら速度を殺す
		}
	} else {
		position = tryPosition;
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

	int screenWidth = static_cast<int>(TuboEngine::WinApp::GetInstance()->GetClientWidth());
	int screenHeight = static_cast<int>(TuboEngine::WinApp::GetInstance()->GetClientHeight());
	int mouseX = static_cast<int>(TuboEngine::Input::GetInstance()->GetMousePosition().x);
	int mouseY = static_cast<int>(TuboEngine::Input::GetInstance()->GetMousePosition().y);
	reticlePosition = TuboEngine::Math::Vector2(static_cast<float>(mouseX), static_cast<float>(mouseY));

	if (IsAutoControlEnabled()) {
		// 自動操作中は移動方向（あるいは敵方向）を向く
		// autoMoveDir_ が 0 でなければ更新
		if (autoMoveDir_.x != 0.0f || autoMoveDir_.y != 0.0f) {
			float angle = std::atan2(autoMoveDir_.x, -autoMoveDir_.y);
			rotation.z = 3.12f + angle;
		}
		return;
	}

	// レイキャストで算出した地面上ターゲット方向で回転を更新（斜め視点対応）
	TuboEngine::Math::Vector3 aimDir = GetAimDirectionFromReticle();
	// 反転補正を削除し、レティクル方向と一致させる
	float angle = std::atan2(aimDir.x, -aimDir.y);
	rotation.z = 3.12f+angle;
}

void Player::ReticleDraw() {
	if (reticleSprite) {
		reticleSprite->Draw();
	}
}
 
//--------------------------------------------------
// 当たり判定の中心座標を取得
//--------------------------------------------------
TuboEngine::Math::Vector3 Player::GetCenterPosition() const {
	const TuboEngine::Math::Vector3 offset = {0.0f, 0.0f, 0.0f};
	TuboEngine::Math::Vector3 worldPosition = position + offset;
	return worldPosition;
}

//--------------------------------------------------
// 衝突時の処理
//--------------------------------------------------
void Player::OnCollision(Collider* other) {
	if (isDodging) {
		return;
	}
   if (!other) {
		return;
	}
	uint32_t typeID = other->GetTypeID();
    if (isInvincible_) {
		return;
	}

	// ダメージはクールダウン中に重ね掛けしない（多段ヒット対策）
	if (damageCooldownTimer > 0.0f) {
		return;
	}

	if (typeID == static_cast<uint32_t>(CollisionTypeId::kEnemy) ||
		typeID == static_cast<uint32_t>(CollisionTypeId::kEnemyWeapon)) {
		HP -= 1;
		isHit = true;
		damageCooldownTimer = damageCooldownTime;
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
	ImGui::Checkbox("Invincible", &isInvincible_);
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
	ImGui::End();
	object3d->DrawImGui("Player");
	PlayerBullet::DrawImGuiGlobal();
#endif // USE_IMGUI
}

// --- 回避開始 ---
void Player::StartDodge() {
	isDodging = true;
	dodgeTimer = dodgeDuration;
	TuboEngine::Math::Vector3 inputDir = GetDodgeInputDirection();
	if (inputDir.x != 0.0f || inputDir.y != 0.0f) {
		float len = std::sqrt(inputDir.x * inputDir.x + inputDir.y * inputDir.y);
		if (len > 0.0f) {
			inputDir.x /= len;
			inputDir.y /= len;
		}
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
		if (dodgeCooldownTimer < 0.0f)
			dodgeCooldownTimer = 0.0f;
	}
	if (isDodging) {
		dodgeTimer -= 1.0f / 60.0f;
		if (dodgeTimer <= 0.0f) {
			isDodging = false;
			dodgeCooldownTimer = dodgeCooldown;
		}
	}
}

// --- 回避入力方向取得 ---
TuboEngine::Math::Vector3 Player::GetDodgeInputDirection() const {
	TuboEngine::Math::Vector3 inputDir(0.0f, 0.0f, 0.0f);
	if (TuboEngine::Input::GetInstance()->PushKey(DIK_W))
		inputDir.y -= 1.0f;
	if (TuboEngine::Input::GetInstance()->PushKey(DIK_S))
		inputDir.y += 1.0f;
	if (TuboEngine::Input::GetInstance()->PushKey(DIK_A))
		inputDir.x -= 1.0f;
	if (TuboEngine::Input::GetInstance()->PushKey(DIK_D))
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
	TuboEngine::Math::Vector2 mouse = TuboEngine::Input::GetInstance()->GetMousePosition();
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
	TuboEngine::Math::Vector3 aim = {hit.x - position.x, hit.y - position.y, hit.z - position.z};
	float ilen = std::sqrt(aim.x * aim.x + aim.y * aim.y + aim.z * aim.z);
	if (ilen > 0.0f) { aim.x /= ilen; aim.y /= ilen; aim.z /= ilen; }
	return aim;
}
