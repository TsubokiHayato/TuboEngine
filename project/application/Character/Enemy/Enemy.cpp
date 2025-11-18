#include "Enemy.h"
#include "Character/Player/Player.h"
#include "Collider/CollisionTypeId.h"
#include "Enemy.h"
#include "ImGuiManager.h"
#include "LineManager.h"
#include "Sprite.h"
#include "TextureManager.h"
#include <algorithm>
#include <cmath>
#include <limits>
#include <queue>
// 追加: プレイヤー弾のダメージを使いたい場合
#include "Bullet/Player/PlayerBullet.h"

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

	HP = 1;

	// Idle背面確認タイマー初期化
	idleLookAroundTimer = idleLookAroundIntervalSec;

	// Idle背面確認フェーズ初期化
	idleBackPhase_ = IdleBackPhase::None;
	idleBackHoldTimer = 0.0f;
	idleBackStartAngle = rotation.z;
	idleBackTargetAngle = rotation.z;
}

// 角度差分を[-π, π]に正規化する関数
static float NormalizeAngle(float angle) {
	while (angle > kPI)
		angle -= 2.0f * kPI;
	while (angle < -kPI)
		angle += 2.0f * kPI;
	return angle;
}

// 矩形（エネミーの当たり）でタイル衝突を考慮して移動
static void MoveWithCollision(Vector3& position, const Vector3& desiredMove, MapChipField* field) {
	if (!field) {
		position = position + desiredMove;
		return;
	}

	const float tile = MapChipField::GetBlockSize();
	const float width = tile * 0.8f;  // 当たり判定幅（調整可）
	const float height = tile * 0.8f; // 当たり判定高さ（調整可）

	// サブステップでトンネリング防止
	float moveLen2D = std::sqrt(desiredMove.x * desiredMove.x + desiredMove.y * desiredMove.y);
	int subSteps = std::max(1, int(std::ceil(moveLen2D / (tile * 0.5f))));
	Vector3 step = desiredMove / float(subSteps);

	for (int i = 0; i < subSteps; ++i) {
		// X軸
		Vector3 nextX = position;
		nextX.x += step.x;
		if (!field->IsRectBlocked(nextX, width, height)) {
			position = nextX;
		}
		// Y軸
		Vector3 nextY = position;
		nextY.y += step.y;
		if (!field->IsRectBlocked(nextY, width, height)) {
			position = nextY;
		}
	}
}

// ---- A* 用ユーティリティ ----
struct AStarCell {
	float g = std::numeric_limits<float>::infinity();
	float f = std::numeric_limits<float>::infinity();
	int parent = -1;
	bool closed = false;
	bool opened = false;
};

static inline int Index1D(int x, int y, int w) { return y * w + x; }

static bool IsTileWalkable(MapChipField* field, uint32_t x, uint32_t y) {
	if (!field)
		return false;
	// 範囲チェック
	if (x >= field->GetNumBlockHorizontal() || y >= field->GetNumBlockVirtical())
		return false;
	// タイル中心座標を取得してブロック判定
	Vector3 center = field->GetMapChipPositionByIndex(x, y);
	return !field->IsBlocked(center);
}

static int FindNearestWalkableIndex(MapChipField* field, int gx, int gy) {
	const int W = (int)field->GetNumBlockHorizontal();
	const int H = (int)field->GetNumBlockVirtical();
	if (gx < 0 || gy < 0 || gx >= W || gy >= H)
		return -1;
	if (IsTileWalkable(field, (uint32_t)gx, (uint32_t)gy))
		return Index1D(gx, gy, W);

	// 半径を広げながら探索（4近傍→リング）
	const int maxRadius = 6;
	for (int r = 1; r <= maxRadius; ++r) {
		for (int dy = -r; dy <= r; ++dy) {
			for (int dx = -r; dx <= r; ++dx) {
				int nx = gx + dx;
				int ny = gy + dy;
				if (nx < 0 || ny < 0 || nx >= W || ny >= H)
					continue;
				if (std::abs(dx) + std::abs(dy) != r)
					continue; // 外周のみ
				if (IsTileWalkable(field, (uint32_t)nx, (uint32_t)ny)) {
					return Index1D(nx, ny, W);
				}
			}
		}
	}
	return -1;
}

bool Enemy::BuildPathTo(const Vector3& worldGoal) {
	currentPath_.clear();
	pathCursor_ = 0;
	lastPathGoalIndex_ = -1;

	if (!mapChipField)
		return false;

	const int W = (int)mapChipField->GetNumBlockHorizontal();
	const int H = (int)mapChipField->GetNumBlockVirtical();
	if (W <= 0 || H <= 0)
		return false;

	// 始点・終点のタイルインデックス
	auto sIdx = mapChipField->GetMapChipIndexSetByPosition(position);
	auto gIdx = mapChipField->GetMapChipIndexSetByPosition(worldGoal);
	int sx = (int)sIdx.xIndex;
	int sy = (int)sIdx.yIndex;
	int gx = (int)gIdx.xIndex;
	int gy = (int)gIdx.yIndex;

	// 範囲内へクランプ
	sx = std::clamp(sx, 0, W - 1);
	sy = std::clamp(sy, 0, H - 1);
	gx = std::clamp(gx, 0, W - 1);
	gy = std::clamp(gy, 0, H - 1);

	// 歩行可能でない終点は最近傍の歩行可タイルへ寄せる
	int gFlat = FindNearestWalkableIndex(mapChipField, gx, gy);
	if (gFlat < 0) {
		return false;
	}
	gx = gFlat % W;
	gy = gFlat / W;
	lastPathGoalIndex_ = gFlat;

	// 始点も歩行可能か確認（不可能なら最近傍の可能タイルへ）
	if (!IsTileWalkable(mapChipField, (uint32_t)sx, (uint32_t)sy)) {
		int sFlat = FindNearestWalkableIndex(mapChipField, sx, sy);
		if (sFlat < 0)
			return false;
		sx = sFlat % W;
		sy = sFlat / W;
	}

	std::vector<AStarCell> grid((size_t)W * (size_t)H);
	auto Heuristic = [&](int x, int y) {
		// マンハッタン（4近傍）
		return float(std::abs(x - gx) + std::abs(y - gy));
	};

	struct Node {
		int idx;
		float f;
	};
	struct Cmp {
		bool operator()(const Node& a, const Node& b) const { return a.f > b.f; }
	};
	std::priority_queue<Node, std::vector<Node>, Cmp> open;

	int sFlat = Index1D(sx, sy, W);
	grid[sFlat].g = 0.0f;
	grid[sFlat].f = Heuristic(sx, sy);
	grid[sFlat].opened = true;
	open.push({sFlat, grid[sFlat].f});

	const int kDir[4][2] = {
	    {1,  0 },
        {-1, 0 },
        {0,  1 },
        {0,  -1}
    };

	int goalFound = -1;
	while (!open.empty()) {
		auto cur = open.top();
		open.pop();
		int cIdx = cur.idx;
		if (grid[cIdx].closed)
			continue;
		grid[cIdx].closed = true;

		int cx = cIdx % W;
		int cy = cIdx / W;

		if (cIdx == lastPathGoalIndex_) {
			goalFound = cIdx;
			break;
		}

		for (auto& d : kDir) {
			int nx = cx + d[0];
			int ny = cy + d[1];
			if (nx < 0 || ny < 0 || nx >= W || ny >= H)
				continue;
			if (!IsTileWalkable(mapChipField, (uint32_t)nx, (uint32_t)ny))
				continue;

			int nIdx = Index1D(nx, ny, W);
			if (grid[nIdx].closed)
				continue;

			float ng = grid[cIdx].g + 1.0f; // 4近傍=コスト1
			if (!grid[nIdx].opened || ng < grid[nIdx].g) {
				grid[nIdx].g = ng;
				grid[nIdx].f = ng + Heuristic(nx, ny);
				grid[nIdx].parent = cIdx;
				if (!grid[nIdx].opened) {
					grid[nIdx].opened = true;
					open.push({nIdx, grid[nIdx].f});
				} else {
					// 既にopenedでも新しいfでpushしておく（lazy update）
					open.push({nIdx, grid[nIdx].f});
				}
			}
		}
	}

	if (goalFound < 0) {
		return false;
	}

	// 経路復元（タイル中心のワールド座標列に変換）
	std::vector<int> rev;
	for (int p = goalFound; p >= 0; p = grid[p].parent) {
		rev.push_back(p);
		if (p == sFlat)
			break;
	}
	std::reverse(rev.begin(), rev.end());

	currentPath_.reserve(rev.size());
	for (int flat : rev) {
		int tx = flat % W;
		int ty = flat / W;
		Vector3 center = mapChipField->GetMapChipPositionByIndex((uint32_t)tx, (uint32_t)ty);
		// Zは現在高度を維持
		center.z = position.z;
		currentPath_.push_back(center);
	}
	pathCursor_ = 0;

	// ウェイポイント到達判定の緩和（ブロックサイズ依存）
	waypointArriveEps_ = std::max(0.1f, MapChipField::GetBlockSize() * 0.25f);
	return !currentPath_.empty();
}

void Enemy::Update() {

	if (HP <= 0) {
		isAllive = false;
		return;
	}

	const float dt = 1.0f / 60.0f; // 60FPS前提

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
		// 目標が変わるので経路は一旦破棄
		ClearPath();
	} else if (lastSeenTimer > 0.0f) {
		lastSeenTimer -= dt;
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

	// --- 射撃条件評価 ---
	wantShoot_ = (canSeePlayer && (state_ == State::Chase || state_ == State::Attack));

	// クールダウンタイマー更新
	if (wantShoot_) {
		bulletTimer_ += dt; // 60FPS前提
	} else {
		bulletTimer_ = std::min(bulletTimer_, EnemyNormalBullet::s_fireInterval);
	}

	auto TryFire = [this]() {
		if (!wantShoot_) { return; }
		if (bullet && bullet->GetIsAlive()) { return; }
		if (bullet && !bullet->GetIsAlive()) { bullet.reset(); }
		if (bulletTimer_ < EnemyNormalBullet::s_fireInterval) { return; }

		bulletTimer_ = 0.0f;

		bullet = std::make_unique<EnemyNormalBullet>();
		bullet->Initialize(position);
		bullet->SetEnemyPosition(position);
		bullet->SetEnemyRotation(rotation);
		bullet->SetPlayer(player_);
		bullet->SetCamera(camera_);
		bullet->SetMapChipField(mapChipField); // 壁衝突用
	};

	// 状態ごとの行動
	switch (state_) {
	case State::Idle: {
		ClearPath();

		// Idle: 一定間隔で背面を向いて、少し保持してから元の向きへ戻る
		switch (idleBackPhase_) {
		case IdleBackPhase::None:
			// 間隔カウント
			idleLookAroundTimer -= dt;
			if (idleLookAroundTimer <= 0.0f) {
				// 背面確認開始
				idleBackStartAngle = rotation.z;
				idleBackTargetAngle = NormalizeAngle(idleBackStartAngle + kPI);
				idleBackHoldTimer = idleBackHoldSec;
				idleBackPhase_ = IdleBackPhase::ToBack;
			}
			break;

		case IdleBackPhase::ToBack: {
			float diff = NormalizeAngle(idleBackTargetAngle - rotation.z);
			float turn = std::clamp(diff, -idleBackTurnSpeed, idleBackTurnSpeed);
			rotation.z = NormalizeAngle(rotation.z + turn);
			if (std::fabs(diff) < 0.01f) {
				rotation.z = idleBackTargetAngle;
				idleBackPhase_ = IdleBackPhase::Hold;
			}
		} break;

		case IdleBackPhase::Hold:
			idleBackHoldTimer -= dt;
			if (idleBackHoldTimer <= 0.0f) {
				idleBackPhase_ = IdleBackPhase::Return;
			}
			break;

		case IdleBackPhase::Return: {
			float diff = NormalizeAngle(idleBackStartAngle - rotation.z);
			float turn = std::clamp(diff, -idleBackTurnSpeed, idleBackTurnSpeed);
			rotation.z = NormalizeAngle(rotation.z + turn);
			if (std::fabs(diff) < 0.01f) {
				rotation.z = idleBackStartAngle;
				idleBackPhase_ = IdleBackPhase::None;
				// 次回のためにタイマーをリセット
				idleLookAroundTimer = idleLookAroundIntervalSec;
			}
		} break;
		}
		break;
	}
	case State::Alert: {
		// Idle背面確認は停止
		idleBackPhase_ = IdleBackPhase::None;

		// ラストスポット（lastSeenPlayerPos）のタイルまでA*で経路追従
		if (mapChipField) {
			auto g = mapChipField->GetMapChipIndexSetByPosition(lastSeenPlayerPos);
			int goalFlat = (int)g.yIndex * (int)mapChipField->GetNumBlockHorizontal() + (int)g.xIndex;
			if (currentPath_.empty() || lastPathGoalIndex_ != goalFlat) {
				BuildPathTo(lastSeenPlayerPos);
			}
		}

		// 経路に沿って移動
		if (!currentPath_.empty() && pathCursor_ < currentPath_.size()) {
			Vector3 waypoint = currentPath_[pathCursor_];
			Vector3 toWP = waypoint - position; toWP.z = 0.0f;
			float len = std::sqrt(toWP.x * toWP.x + toWP.y * toWP.y);
			if (len <= waypointArriveEps_) {
				pathCursor_++;
				if (pathCursor_ >= currentPath_.size()) {
					// ゴール到達
					ClearPath();
					lookAroundInitialized = false;
					state_ = State::LookAround;
				}
			} else {
				Vector3 dir = { toWP.x / (len + 1e-6f), toWP.y / (len + 1e-6f), 0.0f };
				Vector3 desiredMove{ dir.x * (moveSpeed_ * 0.8f), dir.y * (moveSpeed_ * 0.8f), 0.0f };
				MoveWithCollision(position, desiredMove, mapChipField);
			}
		} else {
			// 経路が見つからない場合はその場見回しに移行
			lookAroundInitialized = false;
			state_ = State::LookAround;
		}
		break;
	}
	case State::LookAround: {
		// Idle背面確認は停止
		idleBackPhase_ = IdleBackPhase::None;

		// 既存ロジック
		if (!lookAroundInitialized) {
			lookAroundBaseAngle = rotation.z;
			lookAroundTargetAngle = lookAroundBaseAngle + lookAroundAngleWidth;
			lookAroundDirection = 1;
			lookAroundCount = 0;
			lookAroundInitialized = true;
		}

		float diff = NormalizeAngle(lookAroundTargetAngle - rotation.z);
		float turn = std::clamp(diff, -lookAroundSpeed, lookAroundSpeed);
		rotation.z = NormalizeAngle(rotation.z + turn);

		if (std::fabs(diff) < 0.02f) {
			lookAroundDirection *= -1;
			lookAroundTargetAngle = lookAroundBaseAngle + lookAroundDirection * lookAroundAngleWidth;
			lookAroundCount++;
			if (lookAroundCount >= lookAroundMaxCount) {
				lookAroundInitialized = false;
				state_ = State::Patrol;
			}
		}
		break;
	}
	case State::Patrol: {
		// Idle背面確認は停止
		idleBackPhase_ = IdleBackPhase::None;

		ClearPath();
		state_ = State::Idle;
		break;
	}
	case State::Chase: {
		// Idle背面確認は停止
		idleBackPhase_ = IdleBackPhase::None;

		// プレイヤーを追跡（直接移動 + 衝突回避）
		Vector3 dir = player_->GetPosition() - position;
		dir.z = 0.0f;
		float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
		if (len > 0.1f) {
			dir.x /= len; dir.y /= len;
			Vector3 desiredMove{dir.x * moveSpeed_, dir.y * moveSpeed_, 0.0f};
			MoveWithCollision(position, desiredMove, mapChipField);
		}
		TryFire(); // 追跡中でも射撃
	} break;
	case State::Attack: {
		// Idle背面確認は停止
		idleBackPhase_ = IdleBackPhase::None;

		TryFire(); // 攻撃状態
	} break;
	}

	// 弾更新（生存していれば）
	if (bullet && bullet->GetIsAlive()) {
		bullet->Update();
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
	// null 安全
	if (!other) {
		return;
	}

	// 相手がプレイヤーの武器以外なら無視
	const uint32_t typeID = other->GetTypeID();
	if (typeID != static_cast<uint32_t>(CollisionTypeId::kPlayerWeapon)) {
		return;
	}

	// ここからダメージ処理（プレイヤー武器のみ）
	isHit = true;

	// 弾のグローバルダメージを使う場合:
	HP -= PlayerBullet::s_damage;

	if (HP <= 0) {
		isAllive = false;
	}

	// 発見状態へ（元の挙動を維持）
	if (state_ == State::Idle || state_ == State::Alert) {

		if (player_) {
			lastSeenPlayerPos = player_->GetPosition();
			lastSeenTimer = kLastSeenDuration;
		}
		state_ = State::Chase;
	}
}

void Enemy::Draw() {
	if (isAllive == false) { return; }
	if (object3d) { object3d->Draw(); }
	if (bullet) { bullet->Draw(); }
	DrawViewCone();
	DrawLastSeenMark();
	DrawStateIcon();
}

void Enemy::ParticleDraw() {
	if (particle) { particle->Draw(); }
}

void Enemy::DrawImGui() {

#ifdef USE_IMGUI
	ImGui::Begin("Enemy");
	ImGui::Text("Enemy Position: (%.2f, %.2f, %.2f)", position.x, position.y, position.z);
	ImGui::Text("Enemy Velocity: (%.2f, %.2f, %.2f)", velocity.x, velocity.y, velocity.z);
	ImGui::Text("Enemy HP: %d", HP);
	ImGui::Text("Enemy Alive: %s", isAllive ? "Yes" : "No");
	ImGui::Text("Hit: %s", isHit ? "Yes" : "No");
	ImGui::Text("wasHit: %s", isHit ? "Yes" : "No");
	ImGui::SliderFloat("Turn Speed", &turnSpeed_, 0.01f, 1.0f, "%.2f");

	// --- 見回し（LookAround） ---
	ImGui::SliderFloat("LookAround AngleWidth (rad)", &lookAroundAngleWidth, 0.1f, 2.5f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
	ImGui::SliderFloat("LookAround Speed (rad/frame)", &lookAroundSpeed, 0.005f, 0.08f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
	ImGui::SliderInt("LookAround MaxCount", &lookAroundMaxCount, 1, 10);
	ImGui::Text("LookAround AngleWidth = %.1f deg", lookAroundAngleWidth * 180.0f / kPI);

	// --- Idle: 背面確認 ---
	ImGui::Separator();
	ImGui::SliderFloat("Idle Back Interval (sec)", &idleLookAroundIntervalSec, 0.5f, 10.0f, "%.2f");
	ImGui::SliderFloat("Idle Back TurnSpeed (rad/frame)", &idleBackTurnSpeed, 0.005f, 0.08f, "%.3f");
	ImGui::SliderFloat("Idle Back Hold (sec)", &idleBackHoldSec, 0.0f, 2.0f, "%.2f");
	ImGui::Text("Idle Timer: %.2f", idleLookAroundTimer);

	// --- 状態デバッグ ---
	ImGui::Separator();
	ImGui::Text(
	    "State: %s", state_ == State::Idle         ? "Idle"
	                 : state_ == State::Alert      ? "Alert"
	                 : state_ == State::LookAround ? "LookAround"
	                 : state_ == State::Chase      ? "Chase"
	                 : state_ == State::Attack     ? "Attack"
	                                               : "Unknown");
	ImGui::End();
#endif // USE_IMGUI
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

	// ブロック越し判定（レイサンプル）
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
	for (int i = 0; i < kViewLineDiv; ++i) {
		float a0 = baseAngle - halfRad + (halfRad * 2.0f) * (float(i) / kViewLineDiv);
		Vector3 p0 = center + Vector3{std::cos(a0) * kViewDistance, std::sin(a0) * kViewDistance, 0.0f};
		p0.z = center.z; // 敵の高さで描く
		LineManager::GetInstance()->DrawLine(center, p0, kViewColor);
	}
	for (int i = 0; i < kViewLineDiv - 1; ++i) {
		float a0 = baseAngle - halfRad + (halfRad * 2.0f) * (float(i) / kViewLineDiv);
		float a1 = baseAngle - halfRad + (halfRad * 2.0f) * (float(i + 1) / kViewLineDiv);
		Vector3 p0 = center + Vector3{std::cos(a0) * kViewDistance, std::sin(a0) * kViewDistance, 0.0f};
		Vector3 p1 = center + Vector3{std::cos(a1) * kViewDistance, std::sin(a1) * kViewDistance, 0.0f};
		p0.z = p1.z = center.z; // 敵の高さで描く
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
	// 未使用
}