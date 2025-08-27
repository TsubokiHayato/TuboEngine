#include "Enemy.h"
#include "CollisionTypeId.h"
#include "Enemy.h"
#include "ImGuiManager.h"
#include "LineManager.h"
#include "Player.h"
#include "TextureManager.h"
#include "Sprite.h"
#include <cmath>

constexpr float kPI = 3.14159265358979323846f;

Enemy::Enemy() {}
Enemy::~Enemy() {}

void Enemy::Initialize() {
	// プレイヤーのコライダーの設定
	Collider::SetTypeID(static_cast<uint32_t>(CollisionTypeId::kEnemy));

	// Enemy固有の初期化
	position = {0.0f, 0.0f, 5.0f};
	rotation = {};
	scale = {1.0f, 1.0f, 1.0f}; // 初期スケール

	object3d = std::make_unique<Object3d>();
	const std::string modelFileNamePath = "barrier.obj";
	object3d->Initialize(modelFileNamePath);
	object3d->SetPosition(position);
	object3d->SetRotation(rotation);
	object3d->SetScale(scale);

	std::string particleTextureHandle = "gradationLine.png";
	TextureManager::GetInstance()->LoadTexture(particleTextureHandle);

	// パーティクル関連は初期化しない（ヒット時に生成）
	particle = nullptr;
	particleEmitter_ = nullptr;

	// スプライト初期化
	exclamationSprite_ = std::make_unique<Sprite>();
	exclamationSprite_->Initialize("exclamation.png");
	exclamationSprite_->SetSize({ stateIconSize * 100.0f, stateIconSize * 100.0f });
	exclamationSprite_->SetColor(stateIconColor);
	exclamationSprite_->SetAnchorPoint({0.5f, 0.0f});

	questionSprite_ = std::make_unique<Sprite>();
	questionSprite_->Initialize("question.png");
	questionSprite_->SetSize({ stateIconSize * 100.0f, stateIconSize * 100.0f });
	questionSprite_->SetColor(stateIconColor);
	questionSprite_->SetAnchorPoint({0.5f, 0.0f});
}

// 角度差分を[-π, π]に正規化する関数
static float NormalizeAngle(float angle) {
	while (angle > kPI)
		angle -= 2.0f * kPI;
	while (angle < -kPI)
		angle += 2.0f * kPI;
	return angle;
}
void Enemy::Update() {
	// プレイヤーがいれば距離と方向を計算
	float distanceToPlayer = 0.0f;
	Vector3 toPlayer = {0, 0, 0};
	if (player_) {
		toPlayer = player_->GetPosition() - position;
		distanceToPlayer = std::sqrt(toPlayer.x * toPlayer.x + toPlayer.y * toPlayer.y + toPlayer.z * toPlayer.z);
	}

	// --- 視覚ギミック追加 ---
	bool canSeePlayer = CanSeePlayer();

	// --- 追加: 発見記憶ロジック ---
	if (canSeePlayer) {
		lastSeenPlayerPos = player_->GetPosition();
		lastSeenTimer = kLastSeenDuration;
	} else if (lastSeenTimer > 0.0f) {
		lastSeenTimer -= 1.0f / 60.0f; // 60FPS前提
	}

	// 状態遷移
	if (player_) {
		if (canSeePlayer) {
			if (distanceToPlayer > moveStartDistance_) {
				state_ = State::Idle; // 非発見状態
			} else if (distanceToPlayer > shootDistance_) {
				state_ = State::Chase; // 追跡
			} else {
				state_ = State::Attack; // 攻撃
			}
		} else if (lastSeenTimer > 0.0f) {
			state_ = State::Alert; // 警戒状態（見失った直後）
		} else if (state_ == State::Alert) {
			state_ = State::LookAround; // 警戒タイマー終了後に見回し状態へ
		} else if (state_ != State::LookAround) {
			state_ = State::Idle; // 非発見状態
		}
	}

	// プレイヤーの方向を向く（一定速度で回転）
	if (player_ && (state_ == State::Chase || state_ == State::Attack || state_ == State::Alert)) {
		Vector3 targetPos = (canSeePlayer) ? player_->GetPosition() : lastSeenPlayerPos;
		Vector3 toTarget = targetPos - position;
		float angleZ = std::atan2(toTarget.y, toTarget.x);
		float diff = NormalizeAngle(angleZ - rotation.z);
		float maxTurn = turnSpeed_;
		if (std::fabs(diff) < maxTurn) {
			rotation.z = angleZ;
		} else {
			rotation.z += (diff > 0 ? 1 : -1) * maxTurn;
			rotation.z = NormalizeAngle(rotation.z);
		}
	}

	// 状態ごとの行動
	switch (state_) {
	case State::Idle:
		// 何もしない
		break;
	case State::Alert: {
		// ラストスポットまでの距離を計算
		Vector3 dir = lastSeenPlayerPos - position;
		dir.z = 0.0f;
		float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);

		if (len > 0.05f) {
			float move = moveSpeed_ * 0.5f;
			if (len < move) {
				// 残り距離より移動量が大きい場合はピッタリ到達させる
				position = lastSeenPlayerPos;
			} else {
				dir.x /= len;
				dir.y /= len;
				position.x += dir.x * move;
				position.y += dir.y * move;
			}
		} else {
			// 到達後はその場で回転して見回す
			constexpr float lookAroundSpeed = 0.05f;
			rotation.z += lookAroundSpeed;
			rotation.z = NormalizeAngle(rotation.z);
		}
		break;
	}
	case State::LookAround: {
    // 状態遷移時に初期化
    if (!lookAroundInitialized) {
        lookAroundBaseAngle = rotation.z;
        lookAroundTargetAngle = lookAroundBaseAngle + lookAroundAngleWidth;
        lookAroundDirection = 1;
        lookAroundCount = 0;
        lookAroundInitialized = true;
    }

    float diff = NormalizeAngle(lookAroundTargetAngle - rotation.z);
    float turn = diff;
    if (turn > lookAroundSpeed) turn = lookAroundSpeed;
    if (turn < -lookAroundSpeed) turn = -lookAroundSpeed;
    rotation.z += turn;
    rotation.z = NormalizeAngle(rotation.z);

    // 目標角度に到達したら反対側へ
    if (std::fabs(diff) < 0.02f) {
        lookAroundDirection *= -1;
        lookAroundTargetAngle = lookAroundBaseAngle + lookAroundDirection * lookAroundAngleWidth;
        lookAroundCount++;
        if (lookAroundCount >= lookAroundMaxCount) {
            lookAroundInitialized = false; // 次回のためにリセット
            state_ = State::Patrol; // 巡回モードへ
        }
    }
    break;
	}
	case State::Patrol: {
		// 巡回行動（例: Idleに戻す、または巡回ポイントへ移動など）
		// ここに巡回ロジックを実装
		// 今はIdleに戻すだけ
		state_ = State::Idle;
		break;
	}
	case State::Chase: {
		// プレイヤーを追跡
		Vector3 dir = player_->GetPosition() - position;
		dir.z = 0.0f;
		float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
		if (len > 0.1f) {
			dir.x /= len;
			dir.y /= len;
			position.x += dir.x * moveSpeed_;
			position.y += dir.y * moveSpeed_;
		}
	} break;
	case State::Attack: {
		// 攻撃（弾発射）
		static float bulletTimer = 0.0f;
		bulletTimer += 1.0f / 60.0f;
		if (bulletTimer >= 1.0f) {
			bulletTimer = 0.0f;
			if (player_) {
				bullet = std::make_unique<EnemyNormalBullet>();
				bullet->Initialize(position);
				bullet->SetEnemyPosition(position);
				bullet->SetEnemyRotation(rotation);
				bullet->SetPlayer(player_);
				bullet->SetCamera(camera_);
			}
		}
		if (bullet) {
			bullet->Update();
		}
	} break;
	}

	object3d->SetPosition(position);
	object3d->SetRotation(rotation);
	object3d->SetScale(scale);
	object3d->SetCamera(camera_);
	object3d->Update();

	if (!wasHit && isHit) {
		EmitHitParticle();
	}
	isHit = false;
	wasHit = isHit;

	if (particle) {
		particle->SetCamera(camera_);
		particle->Update();
	}
	if (particleEmitter_) {
		particleEmitter_->Update();
	}
}

void Enemy::EmitHitParticle() {
	// 既存のパーティクルをクリア
	particle = nullptr;
	particleEmitter_ = nullptr;

	std::string particleTextureHandle = "gradationLine.png";
	TextureManager::GetInstance()->LoadTexture(particleTextureHandle);

	particle = std::make_unique<Particle>();
	particle->Initialize(ParticleType::Ring);
	particle->CreateParticleGroup("Particle", particleTextureHandle);

	// 必ず最新のpositionを使う
	particleTranslate = {
	    {1.0f, 1.0f, 1.0f},
        {0.0f, 0.0f, 0.0f},
        position
    };
	particleVelocity = {};
	particleColor = {1.0f, 1.0f, 1.0f, 1.0f};
	particleLifeTime = 1.0f;
	particleCurrentTime = 0.0f;

	particleEmitter_ = std::make_unique<ParticleEmitter>(particle.get(), "Particle", particleTranslate, particleVelocity, particleColor, particleLifeTime, particleCurrentTime, 1, 1.0f, false);
}

void Enemy::OnCollision(Collider* other) {
	// ここで攻撃を受けたかどうかを判定（例: プレイヤーの弾かどうか）
	// 今回は単純に攻撃を受けたら発見状態（Chase）に遷移する例
	isHit = true;
	if (state_ == State::Idle || state_ == State::Alert) {
		// プレイヤーの位置が分かる場合はそこに向かう
		if (player_) {
			lastSeenPlayerPos = player_->GetPosition();
			lastSeenTimer = kLastSeenDuration;
		}
		state_ = State::Chase;
	}
}

void Enemy::Draw() {
	if (object3d) {
		object3d->Draw();
	}
	if (bullet) {
		bullet->Draw();
	}
	DrawViewCone();
	DrawLastSeenMark();
	DrawStateIcon();
}

void Enemy::ParticleDraw() {
	if (particle) {
		particle->Draw();
	}
}

void Enemy::DrawImGui() {
	ImGui::Begin("Enemy");
	ImGui::Text("Enemy Position: (%.2f, %.2f, %.2f)", position.x, position.y, position.z);
	ImGui::Text("Enemy Velocity: (%.2f, %.2f, %.2f)", velocity.x, velocity.y, velocity.z);
	ImGui::Text("Enemy HP: %d", HP);
	ImGui::Text("Enemy Alive: %s", isAlive ? "Yes" : "No");
	ImGui::Text("Hit: %s", isHit ? "Yes" : "No");
	ImGui::Text("wasHit: %s", isHit ? "Yes" : "No");
	ImGui::SliderFloat("Turn Speed", &turnSpeed_, 0.01f, 1.0f, "%.2f");

	// --- 見回しパラメータ ---
	ImGui::SliderFloat("LookAround AngleWidth (deg)", &lookAroundAngleWidth, 0.1f, 2.5f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
	ImGui::SliderFloat("LookAround Speed", &lookAroundSpeed, 0.01f, 0.2f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
	ImGui::SliderInt("LookAround MaxCount", &lookAroundMaxCount, 1, 10);
	ImGui::Text("LookAround AngleWidth = %.1f deg", lookAroundAngleWidth * 180.0f / kPI);

	// --- 状態デバッグ ---
	ImGui::Separator();
	ImGui::Text("State: %s", 
		state_ == State::Idle ? "Idle" :
		state_ == State::Alert ? "Alert" :
		state_ == State::LookAround ? "LookAround" :
		state_ == State::Chase ? "Chase" :
		state_ == State::Attack ? "Attack" : "Unknown");
	if (state_ == State::Alert) {
		Vector3 toLast = lastSeenPlayerPos - position;
		toLast.z = 0.0f;
		float len = std::sqrt(toLast.x * toLast.x + toLast.y * toLast.y);
		bool reached = (len <= 0.05f);
		ImGui::Text("Alert: ReachedLastSpot = %s", reached ? "Yes" : "No");
		ImGui::Text("Alert: lastSeenTimer = %.2f", lastSeenTimer);
		ImGui::Text("Alert: LookAround = %s", reached ? "Yes" : "No");
	}
	ImGui::ColorEdit4("StateIcon Color", &stateIconColor.x);
	ImGui::SliderFloat("StateIcon Size", &stateIconSize, 0.2f, 2.0f);
	ImGui::SliderFloat("StateIcon Height", &stateIconHeight, 1.0f, 6.0f);
	ImGui::SliderFloat("StateIcon LineWidth", &stateIconLineWidth, 0.01f, 0.3f);
	ImGui::End();
}

void Enemy::Move() {}

bool Enemy::CanSeePlayer() {
	if (!player_ || !mapChipField)
		return false;

	Vector3 from = position;
	Vector3 to = player_->GetPosition();
	Vector3 dirToPlayer = to - from;
	dirToPlayer.z = 0.0f;

	// プレイヤーまでの距離
	float distance = std::sqrt(dirToPlayer.x * dirToPlayer.x + dirToPlayer.y * dirToPlayer.y);
	if (distance > kViewDistance)
		return false; // 視認距離外

	// 視野角判定
	Vector3 forward = {std::cos(rotation.z), std::sin(rotation.z), 0.0f}; // Z軸回転
	float dot = Vector3::Dot(Vector3::Normalize(forward), Vector3::Normalize(dirToPlayer));
	float angleToPlayer = std::acos(dot) * 180.0f / kPI;
	if (angleToPlayer > kViewAngleDeg / 2.0f)
		return false; // 視野外

	// ブロック越し判定（従来通り）
	Vector2 start = {from.x, from.y};
	Vector2 end = {to.x, to.y};
	const float step = 0.5f;
	Vector2 dir = end - start;
	float length = std::sqrt(dir.x * dir.x + dir.y * dir.y);
	if (length < 0.001f)
		return true;
	dir.x /= length;
	dir.y /= length;
	for (float t = 0.0f; t <= length; t += step) {
		Vector2 pos = start + dir * t;
		Vector3 checkPos = {pos.x, pos.y, from.z};
		if (mapChipField->IsBlocked(checkPos)) {
			return false;
		}
	}
	return true;
}

void Enemy::DrawViewCone() {
	float halfRad = (kViewAngleDeg / 2.0f) * kPI / 180.0f;
	float baseAngle = rotation.z;
	Vector3 center = position;
	// 中心から端への線はそのまま
	for (int i = 0; i < kViewLineDiv; ++i) {
		float a0 = baseAngle - halfRad + (halfRad * 2.0f) * (float(i) / kViewLineDiv);
		Vector3 p0 = center + Vector3{std::cos(a0) * kViewDistance, std::sin(a0) * kViewDistance, 0.0f};
		LineManager::GetInstance()->DrawLine(center, p0, kViewColor);
	}
	// 外周の弧は分割数-1回だけ描画
	for (int i = 0; i < kViewLineDiv - 1; ++i) {
		float a0 = baseAngle - halfRad + (halfRad * 2.0f) * (float(i) / kViewLineDiv);
		float a1 = baseAngle - halfRad + (halfRad * 2.0f) * (float(i + 1) / kViewLineDiv);
		Vector3 p0 = center + Vector3{std::cos(a0) * kViewDistance, std::sin(a0) * kViewDistance, 0.0f};
		Vector3 p1 = center + Vector3{std::cos(a1) * kViewDistance, std::sin(a1) * kViewDistance, 0.0f};
		LineManager::GetInstance()->DrawLine(p0, p1, kViewColor);
	}
}

Vector3 Enemy::GetCenterPosition() const {
	const Vector3 offset = {0.0f, 0.0f, 0.0f};
	Vector3 worldPosition = position + offset;
	return worldPosition;
}

// --- 追加: ラストスポット描画関数 ---
void Enemy::DrawLastSeenMark() {
	if (lastSeenTimer <= 0.0f)
		return;

	constexpr Vector4 kLastSeenColor = {1.0f, 0.2f, 0.2f, 1.0f}; // 赤色
	constexpr float kLastSeenMarkSize = 0.5f;                    // マークの大きさ
	const Vector3& center = lastSeenPlayerPos;

	// 十字マーク
	LineManager::GetInstance()->DrawLine(center + Vector3{-kLastSeenMarkSize, 0.0f, 0.0f}, center + Vector3{kLastSeenMarkSize, 0.0f, 0.0f}, kLastSeenColor);
	LineManager::GetInstance()->DrawLine(center + Vector3{0.0f, -kLastSeenMarkSize, 0.0f}, center + Vector3{0.0f, kLastSeenMarkSize, 0.0f}, kLastSeenColor);

	// 円（ターゲットマーク外周）
	constexpr int circleDiv = 16;
	for (int i = 0; i < circleDiv; ++i) {
		float a0 = (2.0f * kPI) * (float(i) / circleDiv);
		float a1 = (2.0f * kPI) * (float(i + 1) / circleDiv);
		Vector3 p0 = center + Vector3{std::cos(a0) * kLastSeenMarkSize, std::sin(a0) * kLastSeenMarkSize, 0.0f};
		Vector3 p1 = center + Vector3{std::cos(a1) * kLastSeenMarkSize, std::sin(a1) * kLastSeenMarkSize, 0.0f};
		LineManager::GetInstance()->DrawLine(p0, p1, kLastSeenColor);
	}
}

void Enemy::DrawStateIcon() {
    Vector3 base = position + Vector3{0, 0, stateIconHeight};
    float size = stateIconSize;
    Vector4 color = stateIconColor;

    if (showSurpriseIcon_) {
        // 左に「？」、右に「！」
        if (questionSprite_) {
            questionSprite_->SetPosition({ base.x - 0.4f * size * 100.0f, base.y + base.z * 20.0f });
            questionSprite_->SetColor(color);
            questionSprite_->Draw();
        }
        if (exclamationSprite_) {
            exclamationSprite_->SetPosition({ base.x + 0.4f * size * 100.0f, base.y + base.z * 20.0f });
            exclamationSprite_->SetColor(color);
            exclamationSprite_->Draw();
        }
    } else if (state_ == State::Chase) {
        if (exclamationSprite_) {
            exclamationSprite_->SetPosition({ base.x, base.y + base.z * 20.0f });
            exclamationSprite_->SetColor(color);
            exclamationSprite_->Draw();
        }
    } else if (state_ == State::Alert || state_ == State::LookAround) {
        if (questionSprite_) {
            questionSprite_->SetPosition({ base.x, base.y + base.z * 20.0f });
            questionSprite_->SetColor(color);
            questionSprite_->Draw();
        }
    }
}

// 太いビックリマーク
//void Enemy::DrawExclamationMark(const Vector3& pos, float size, const Vector4& color, float width) {}
// 太いはてなマーク
//void Enemy::DrawQuestionMark(const Vector3& pos, float size, const Vector4& color, float width) {}
