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
#include "Bullet/Player/PlayerCircusBullet.h"
#include "Character/Enemy/CircusEnemy.h"
#include "Bullet/Enemy/CircusBullet.h"
#include "PlayerEvasion.h"
#include "engine/graphic/2d/TextManager.h"
#include <random>

//--------------------------------------------------
// コンストラクタ
//--------------------------------------------------
Player::Player()
    : cooldownTime(0.2f), damageCooldownTimer(0.0f), damageCooldownTime(1.0f) {
    autoController_.Initialize(this);
    evasionManager_ = std::make_unique<PlayerEvasion>();
}

//--------------------------------------------------
// デストラクタ
//--------------------------------------------------
Player::~Player() {
    if (justEvasionText_) {
        TuboEngine::TextManager::GetInstance()->RemoveText(justEvasionText_);
        justEvasionText_ = nullptr;
    }
}

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

	// ジャスト回避やダッシュのUI/ポストエフェクト状態のリセット
	if (justEvasionText_) {
		TuboEngine::TextManager::GetInstance()->RemoveText(justEvasionText_);
	}
	justEvasionText_ = nullptr;
	justEvasionTextTimer_ = 0.0f;
	
	dashPostEffectTimer_ = 0.0f;
	OffScreenRendering::GetInstance()->SetDashPostEffectEnabled(false);
	OffScreenRendering::GetInstance()->SetDashRadialBlurPower(0.02f);
	
	// 回避ステートの完全リセット
	evasionManager_ = std::make_unique<PlayerEvasion>();
	
	// 弾のクリア
	bullets.clear();

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
			// オート操作中: autoAimTarget_ → autoAimDir_ をスムーズ補間
			// autoAimDir_ は AutoController が毎フレーム SetAutoAimDirection で更新した値を
			// ターゲットとして内部補間する
			constexpr float kAimLerpSpeed = 8.0f; // 大きいほど素早く追従
			float dt = 1.0f / 60.0f;
			// autoAimDir_ をそのまま表示用に使うため、実際の目標は autoAimTarget_
			// (SetAutoAimDirection を autoAimTarget_ への書き込みに変更済み)
			// lerp: current + (target - current) * speed * dt
			float lx = autoAimDir_.x + (autoAimTarget_.x - autoAimDir_.x) * kAimLerpSpeed * dt;
			float ly = autoAimDir_.y + (autoAimTarget_.y - autoAimDir_.y) * kAimLerpSpeed * dt;
			float len = std::sqrt(lx * lx + ly * ly);
			if (len > 0.001f) {
				autoAimDir_.x = lx / len;
				autoAimDir_.y = ly / len;
			}
			float angle = std::atan2(autoAimDir_.x, -autoAimDir_.y);
			rotation.z = 3.12f + angle;
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
		ClearBullets();
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

	// 回避開始時のエフェクト処理は UpdateDodge や StartDodge に移動済み

	// Dashポストエフェクトの時間経過で自動復帰
	if (dashPostEffectTimer_ > 0.0f) {
		dashPostEffectTimer_ -= 1.0f / 60.0f;
		// 0→1 の進行度（開始直後=1、終了直前=0）
		float t = dashPostEffectTimer_ / std::max(0.0001f, dashPostEffectDurationCurrent_);
		t = std::clamp(t, 0.0f, 1.0f);
		// 立ち上がりで強く、徐々に弱まる（イージング）
		float eased = t * t; // ease-out
		OffScreenRendering::GetInstance()->SetDashRadialBlurPower(dashRadialBlurPowerCurrentMax_ * eased);
		if (dashPostEffectTimer_ <= 0.0f) {
			dashPostEffectTimer_ = 0.0f;
			OffScreenRendering::GetInstance()->SetDashRadialBlurPower(0.02f); // RadialBlurのデフォルトへ戻す
			OffScreenRendering::GetInstance()->SetDashPostEffectEnabled(false);
		}
	}

	// ジャスト回避テキストの更新とフェードアウト
	if (justEvasionTextTimer_ > 0.0f) {
		justEvasionTextTimer_ -= 1.0f / 60.0f;
		if (justEvasionTextTimer_ <= 0.0f) {
			if (justEvasionText_) {
				TuboEngine::TextManager::GetInstance()->RemoveText(justEvasionText_);
				justEvasionText_ = nullptr;
			}
		} else if (justEvasionText_) {
			// 徐々に上にスクロールしながらフェードアウト
			float progress = 1.0f - (justEvasionTextTimer_ / 1.0f);
			float screenW = static_cast<float>(TuboEngine::WinApp::GetInstance()->GetClientWidth());
			float screenH = static_cast<float>(TuboEngine::WinApp::GetInstance()->GetClientHeight());
			
			TuboEngine::Math::Vector2 textPos = { screenW * 0.5f, screenH * 0.32f - progress * 90.0f };
			justEvasionText_->SetPosition(textPos);
			
			TuboEngine::Math::Vector4 color = { 0.0f, 0.9f, 1.0f, justEvasionTextTimer_ / 1.0f };
			justEvasionText_->SetColor(color);
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
	if (evasionManager_->IsDodging()) {
		TuboEngine::Math::Vector3 dDir = evasionManager_->GetDodgeDirection();
		TuboEngine::Math::Vector3 nextPos = position;
		nextPos.x += dDir.x * evasionManager_->GetDodgeSpeed();
		nextPos.y += dDir.y * evasionManager_->GetDodgeSpeed();
		if (mapChipField) {
			float playerWidth = scale.x * MapChipField::GetBlockWidth() - 0.1f;
			float playerHeight = scale.y * MapChipField::GetBlockHeight() - 0.1f;
			if (!mapChipField->IsRectBlocked(nextPos, playerWidth, playerHeight)) {
				position = nextPos;
			} else {
				velocity = {0.0f, 0.0f, 0.0f};
				return;
			}
		} else {
			position = nextPos;
		}
		// 回避中は速度を更新しておく（回避終了後の慣性のために）
		velocity = dDir * evasionManager_->GetDodgeSpeed();
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
   if (!other) {
		return;
	}
	uint32_t typeID = other->GetTypeID();
    if (isInvincible_) {
		return;
	}

	// 回避中（通常回避無敵・ジャスト回避）の判定を優先。
	// 被弾後のダメージクールダウン中であっても、能動的な回避アクションに対するジャスト判定は有効にします。
	if (typeID == static_cast<uint32_t>(CollisionTypeId::kEnemy) ||
		typeID == static_cast<uint32_t>(CollisionTypeId::kEnemyWeapon)) {
        if (evasionManager_->IsDodging()) {
            if (evasionManager_->TryTriggerJustEvasion()) {
                OnJustEvasion(other);
                return; // ジャスト回避成功でダメージ無効
            }
            return; // 通常の回避中（無敵）
        }
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
	ImGui::Text("Dodge: %s", evasionManager_->IsDodging() ? "Dodging" : "Ready");
	ImGui::Text("Dodge Timer: %.3f sec", evasionManager_->GetDodgeTimer());
	ImGui::Text("Dodge Cooldown: %.3f sec", evasionManager_->GetDodgeCooldownTimer());
	ImGui::Text("Just Evasion Timer: %.3f sec", evasionManager_->GetJustEvasionTimer());
	ImGui::Text("Has Just Evaded: %s", evasionManager_->HasJustEvaded() ? "Yes" : "No");
 ImGui::SeparatorText("Evasion Tuning");
	float justWindow = evasionManager_->GetJustEvasionWindow();
	if (ImGui::DragFloat("Just Window (sec)", &justWindow, 0.01f, 0.0f, 5.0f)) {
		evasionManager_->SetJustEvasionWindow(justWindow);
	}
	float dodgeDuration = evasionManager_->GetDodgeDuration();
	if (ImGui::DragFloat("Dodge Duration (sec)", &dodgeDuration, 0.01f, 0.01f, 2.0f)) {
		evasionManager_->SetDodgeDuration(dodgeDuration);
	}
	float dodgeCooldown = evasionManager_->GetDodgeCooldown();
	if (ImGui::DragFloat("Dodge Cooldown (sec)", &dodgeCooldown, 0.01f, 0.0f, 5.0f)) {
		evasionManager_->SetDodgeCooldown(dodgeCooldown);
	}
	float dodgeSpeed = evasionManager_->GetDodgeSpeed();
	if (ImGui::DragFloat("Dodge Speed", &dodgeSpeed, 0.01f, 0.0f, 5.0f)) {
		evasionManager_->SetDodgeSpeed(dodgeSpeed);
	}
	ImGui::Separator();
	TuboEngine::Math::Vector3 dDir = evasionManager_->GetDodgeDirection();
	ImGui::Text("Dodge Direction: (%.2f, %.2f)", dDir.x, dDir.y);
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
	TuboEngine::Math::Vector3 inputDir = GetDodgeInputDirection();
	evasionManager_->StartDodge(inputDir);
	
	// エフェクト発火
	if (evasionManager_->IsDodging()) {
		TriggerDashRing();
		// Dash演出: ポストエフェクトを一時的にRadialBlurへ
		dashPostEffectTimer_ = dashPostEffectDuration_;
		dashPostEffectDurationCurrent_ = dashPostEffectDuration_;
		dashRadialBlurPowerCurrentMax_ = dashRadialBlurPower_;
		OffScreenRendering::GetInstance()->SetDashPostEffectEnabled(true);
	}
}

// --- 回避状態更新 ---
void Player::UpdateDodge() {
    bool wasDodgingPrevLocal = evasionManager_->IsDodging();

    evasionManager_->Update();

    bool dodgingNow = evasionManager_->IsDodging();

	// 回避エフェクト管理（残像などが必要ならここに）
	if (dashRingEmitter_ && dodgingNow && !wasDodgingPrevLocal) {
		TriggerDashRing();
		// Dash演出: ポストエフェクトを一時的にRadialBlurへ
		dashPostEffectTimer_ = dashPostEffectDuration_;
		dashPostEffectDurationCurrent_ = dashPostEffectDuration_;
		dashRadialBlurPowerCurrentMax_ = dashRadialBlurPower_;
		OffScreenRendering::GetInstance()->SetDashPostEffectEnabled(true);
	}
}

bool Player::CanDodge() const {
    // 自身が死亡状態などでなければ、EvasionManagerの状態で判定
    return (HP > 0 && !evasionManager_->IsDodging());
}

// --- AI方向指定ダッシュ ---
void Player::AutoStartDodgeDir(const TuboEngine::Math::Vector3& dir) {
	evasionManager_->StartDodge(dir);
	// エフェクト発火
	if (evasionManager_->IsDodging()) {
		TriggerDashRing();
		dashPostEffectTimer_ = dashPostEffectDuration_;
		dashPostEffectDurationCurrent_ = dashPostEffectDuration_;
		dashRadialBlurPowerCurrentMax_ = dashRadialBlurPower_;
		OffScreenRendering::GetInstance()->SetDashPostEffectEnabled(true);
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

// --- ジャスト回避成功処理 ---
void Player::OnJustEvasion(Collider* other) {
    if (dashRingEmitter_) {
        dashRingEmitter_->Emit(45); // エフェクト増し増し
    }
    
    // ド派手な画面エフェクト（RadialBlurを強く長くかける）
    OffScreenRendering::GetInstance()->SetDashPostEffectEnabled(true);
    dashPostEffectTimer_ = 0.6f;
    dashPostEffectDurationCurrent_ = 0.6f;
    dashRadialBlurPowerCurrentMax_ = 0.6f; // 強烈にする

    // 既存のジャスト回避テキストがあれば一旦消す
    if (justEvasionText_) {
        TuboEngine::TextManager::GetInstance()->RemoveText(justEvasionText_);
        justEvasionText_ = nullptr;
    }

    // スクリーン中央の少し上に "JUST EVASION!" と表示
    float screenW = static_cast<float>(TuboEngine::WinApp::GetInstance()->GetClientWidth());
    float screenH = static_cast<float>(TuboEngine::WinApp::GetInstance()->GetClientHeight());
    
    // 爽快感のあるシアンカラーで表示
    TuboEngine::Math::Vector2 textPos = { screenW * 0.5f, screenH * 0.32f };
    TuboEngine::Math::Vector4 textColor = { 0.0f, 0.9f, 1.0f, 1.0f }; // シアンブルー
    
    std::string fontName = TuboEngine::TextManager::PresetFontNames::YasashisaGothicBold;
    
    justEvasionText_ = TuboEngine::TextManager::GetInstance()->CreateText(
        fontName,
        "JUST EVASION!",
        textPos,
        textColor,
        1.6f // 大きめに表示
    );
    
    if (justEvasionText_) {
        justEvasionText_->SetHorizontalAlign(1); // Center
        justEvasionText_->SetVerticalAlign(1);   // Middle
    }
    
    justEvasionTextTimer_ = 1.0f; // 1秒間表示

    bool isCircus = false;
    CircusEnemy* boss = nullptr;
    
    if (dynamic_cast<CircusEnemy*>(other) != nullptr) {
        isCircus = true;
    } else if (dynamic_cast<CircusBullet*>(other) != nullptr) {
        isCircus = true;
    }
    
    // 現在の敵リストからサーカスエネミーを探して弾を消去する
    for (auto* enemy : autoController_.GetEnemyList()) {
        if (auto* ce = dynamic_cast<CircusEnemy*>(enemy)) {
            boss = ce;
            isCircus = true;
            // ジャスト回避の恩恵として画面上のサーカス弾を全滅させる
            boss->ClearAllBullets();
        }
    }

    if (isCircus) {
        // サーカス弾を撃ち返すカウンター
        int counterCount = 12; // 12発の一斉射撃
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<float> distAngle(0.0f, 6.283185f);
        std::uniform_real_distribution<float> distSpeed(0.5f, 1.2f);
        
        for (int i = 0; i < counterCount; ++i) {
            auto bullet = std::make_unique<PlayerCircusBullet>();
            bullet->SetPlayerRotation(rotation);
            
            TuboEngine::Math::Vector3 spawnPos = position;
            spawnPos.z += 1.5f; // ちょっと上から出す
            
            bullet->SetPlayerPosition(spawnPos);
            bullet->SetMapChipField(mapChipField);
            
            // 敵リストを渡す
            bullet->SetTargetEnemyList(autoController_.GetEnemyList());
            
            // PlayerCircusBullet専用の初期化（カメラも渡して内部でUpdateさせる）
            bullet->InitializeCircus(spawnPos, object3d->GetCamera(), boss);
            
            // 放射状にランダムな初速を与える
            float angle = distAngle(gen);
            float speed = distSpeed(gen);
            TuboEngine::Math::Vector3 vel = { std::cos(angle) * speed * 5.0f, std::sin(angle) * speed * 5.0f, speed * 6.0f };
            bullet->SetInitialVelocity(vel);
            
            bullets.push_back(std::move(bullet));
        }
    }
}

