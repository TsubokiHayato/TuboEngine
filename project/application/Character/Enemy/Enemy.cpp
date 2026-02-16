#include "Enemy.h"
#include "Bullet/Player/PlayerBullet.h"
#include "Character/Player/Player.h"
#include "Collider/CollisionTypeId.h"
#include "ImGuiManager.h"
#include "LineManager.h"
#include "Sprite.h"
#include "TextureManager.h"
#include <algorithm>
#include <cmath>
#include <limits>
#include <queue>
#include "engine/graphic/Particle/ParticleManager.h"
#include "engine/graphic/Particle/Effects/Primitive/PrimitiveEmitter.h"
#include "engine/graphic/Particle/Effects/Ring/RingEmitter.h"

constexpr float kPI = 3.14159265358979323846f;

// Enemyモデルの軸補正（モデルの向きがゲーム座標系と一致しない場合のための定数）
// 90度ズレが「Z軸(ヨー)」ではなく「X軸/ Y軸」で起きるケースがあるため、軸ごとに分けて補正する。
// ※右ねじの向き(+回転)やモデルの前方軸に合わせて調整してください。
constexpr float kEnemyModelRotOffsetX = 0.0f;
constexpr float kEnemyModelRotOffsetY = 0.0f;
constexpr float kEnemyModelRotOffsetZ = -kPI * 0.5f; // 従来: yaw補正

Enemy::Enemy() {}
Enemy::~Enemy() {}

void Enemy::Initialize() {
    Collider::SetTypeID(static_cast<uint32_t>(CollisionTypeId::kEnemy));
    position_ = {0.0f, 0.0f, 5.0f};
	rotation_ = TuboEngine::Math::Vector3(1.56f, 0.0f, 0.0f);
    scale_ = {1.0f, 1.0f, 1.0f};

    object3d_ = std::make_unique<Object3d>();
    const std::string modelFileNamePath = "player/player.obj";
    object3d_->Initialize(modelFileNamePath);
    object3d_->SetPosition(position_);
    // 初期化時にも描画補正を適用
    {
		TuboEngine::Math::Vector3 drawRot = rotation_;
        drawRot.x = drawRot.x + kEnemyModelRotOffsetX;
        drawRot.y = drawRot.y + kEnemyModelRotOffsetY;
        drawRot.z = drawRot.z + kEnemyModelRotOffsetZ;
        object3d_->SetRotation(drawRot);
    }
    object3d_->SetScale(scale_);
	object3d_->SetModelColor({1.0f, 0.0f, 0.0f, 1.0f});

    std::string particleTextureHandle = "gradationLine.png";
    TextureManager::GetInstance()->LoadTexture(particleTextureHandle);

    // Icons
    TextureManager::GetInstance()->LoadTexture("Question.png");
    TextureManager::GetInstance()->LoadTexture("Exclamation.png");
    questionIcon_ = std::make_unique<Sprite>();
    questionIcon_->Initialize("Question.png");
    questionIcon_->SetGetIsAdjustTextureSize(false);
    questionIcon_->SetAnchorPoint({0.5f, 0.5f});
    exclamationIcon_ = std::make_unique<Sprite>();
    exclamationIcon_->Initialize("Exclamation.png");
    exclamationIcon_->SetGetIsAdjustTextureSize(false);
    exclamationIcon_->SetAnchorPoint({0.5f, 0.5f});

    hp_ = 10; // 調整: 少し耐える

    idleLookAroundTimer_ = idleLookAroundIntervalSec_;
    idleBackPhase_ = IdleBackPhase::None;
    idleBackHoldTimer_ = 0.0f;
    idleBackStartAngle_ = rotation_.z;
    idleBackTargetAngle_ = rotation_.z;

    // ノックバック初期化
    knockbackTimer_ = 0.0f;
    knockbackVelocity_ = {0.0f, 0.0f, 0.0f};

    // --- 追加: 演出用エミッタ生成 ---
    // ヒット時: 小さなスパーク（既存）
    if (!hitEmitter_) {
        ParticlePreset p{};
        p.name = "EnemyHit";
        p.texture = "particle.png";
        p.maxInstances = 64;
        p.autoEmit = false;
        p.burstCount = 12;
        p.lifeMin = 0.25f;
        p.lifeMax = 0.35f;
        p.scaleStart = {0.15f, 0.15f, 0.15f};
        p.scaleEnd = {0.05f, 0.05f, 0.05f};
        p.colorStart = {1.0f, 0.6f, 0.2f, 0.9f};
        p.colorEnd = {1.0f, 1.0f, 0.4f, 0.0f};
        p.center = position_;
        hitEmitter_ = ParticleManager::GetInstance()->CreateEmitter<PrimitiveEmitter>(p);
    }
    // 追加: ヒット時の小さなリングを別エミッタで生成（既存と併用）
    if (!hitRingEmitter_) {
        ParticlePreset p{};
        p.name = "EnemyHitRing";
        p.texture = "gradationLine.png";
        p.maxInstances = 16;
        p.autoEmit = false;
        p.burstCount = 1;
        p.lifeMin = 0.25f;
        p.lifeMax = 0.45f;
        p.scaleStart = {0.4f, 0.4f, 1.0f};
        p.scaleEnd   = {0.4f, 0.4f, 1.0f};
        p.colorStart = {1.0f, 0.5f, 0.2f, 0.9f};
        p.colorEnd   = {1.0f, 0.9f, 0.6f, 0.0f};
        p.center = position_;
        hitRingEmitter_ = ParticleManager::GetInstance()->CreateEmitter<RingEmitter>(p);
    }
    // 死亡時: 大きなリング (一度だけ)
    if (!deathEmitter_) {
        ParticlePreset p{};
        p.name = "EnemyDeath";
        p.texture = "gradationLine.png";
        p.maxInstances = 8;
        p.autoEmit = false;
        p.burstCount = 1;
        p.lifeMin = 0.6f;
        p.lifeMax = 0.9f;
        p.scaleStart = {0.6f, 0.6f, 0.6f};
        p.scaleEnd = {1.4f, 1.4f, 1.4f};
        p.colorStart = {1.0f, 0.3f, 0.2f, 0.9f};
        p.colorEnd = {1.0f, 0.8f, 0.1f, 0.0f};
        p.center = position_;
        deathEmitter_ = ParticleManager::GetInstance()->CreateEmitter<RingEmitter>(p);
    }
}

static float NormalizeAngle(float angle) {
    while (angle > kPI)
        angle -= 2.0f * kPI;
    while (angle < -kPI)
        angle += 2.0f * kPI;
    return angle;
}

static void MoveWithCollision(TuboEngine::Math::Vector3& position, const TuboEngine::Math::Vector3& desiredMove, MapChipField* field) {
    if (!field) {
        position = position + desiredMove;
        return;
    }
    const float tile = MapChipField::GetBlockSize();
    const float width = tile * 0.8f;
    const float height = tile * 0.8f;
    float moveLen2D = std::sqrt(desiredMove.x * desiredMove.x + desiredMove.y * desiredMove.y);
    int subSteps = std::max(1, int(std::ceil(moveLen2D / (tile * 0.5f))));
	TuboEngine::Math::Vector3 step = desiredMove / float(subSteps);
    for (int i = 0; i < subSteps; ++i) {
		TuboEngine::Math::Vector3 nextX = position;
        nextX.x += step.x;
        if (!field->IsRectBlocked(nextX, width, height)) {
            position = nextX;
        }
		TuboEngine::Math::Vector3 nextY = position;
        nextY.y += step.y;
        if (!field->IsRectBlocked(nextY, width, height)) {
            position = nextY;
        }
    }
}

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
    if (x >= field->GetNumBlockHorizontal() || y >= field->GetNumBlockVirtical())
        return false;
	TuboEngine::Math::Vector3 center = field->GetMapChipPositionByIndex(x, y);
    return !field->IsBlocked(center);
}
static int FindNearestWalkableIndex(MapChipField* field, int gx, int gy) {
    const int W = (int)field->GetNumBlockHorizontal();
    const int H = (int)field->GetNumBlockVirtical();
    if (gx < 0 || gy < 0 || gx >= W || gy >= H)
        return -1;
    if (IsTileWalkable(field, (uint32_t)gx, (uint32_t)gy))
        return Index1D(gx, gy, W);
    const int maxRadius = 6;
    for (int r = 1; r <= maxRadius; ++r) {
        for (int dy = -r; dy <= r; ++dy) {
            for (int dx = -r; dx <= r; ++dx) {
                int nx = gx + dx;
                int ny = gy + dy;
                if (nx < 0 || ny < 0 || nx >= W || ny >= H)
                    continue;
                if (std::abs(dx) + std::abs(dy) != r)
                    continue;
                if (IsTileWalkable(field, (uint32_t)nx, (uint32_t)ny)) {
                    return Index1D(nx, ny, W);
                }
            }
        }
    }
    return -1;
}

bool Enemy::PreparePathfinding_(const TuboEngine::Math::Vector3& worldGoal, int& outW, int& outH, int& outSX, int& outSY, int& outGX, int& outGY) {
	currentPath_.clear();
	pathCursor_ = 0;
	lastPathGoalIndex_ = -1;

	if (!mapChipField_) {
		return false;
	}

	outW = (int)mapChipField_->GetNumBlockHorizontal();
	outH = (int)mapChipField_->GetNumBlockVirtical();
	if (outW <= 0 || outH <= 0) {
		return false;
	}

	auto sIdx = mapChipField_->GetMapChipIndexSetByPosition(position_);
	auto gIdx = mapChipField_->GetMapChipIndexSetByPosition(worldGoal);
	outSX = std::clamp((int)sIdx.xIndex, 0, outW - 1);
	outSY = std::clamp((int)sIdx.yIndex, 0, outH - 1);
	outGX = std::clamp((int)gIdx.xIndex, 0, outW - 1);
	outGY = std::clamp((int)gIdx.yIndex, 0, outH - 1);
	return true;
}

bool Enemy::RunAStar_(int W, int H, int sx, int sy, int gFlat, std::vector<int>& outPathFlats) {
	outPathFlats.clear();
	std::vector<AStarCell> grid((size_t)W * (size_t)H);

	int gx = gFlat % W;
	int gy = gFlat / W;
	auto Heuristic = [&](int x, int y) {
		return float(std::abs(x - gx) + std::abs(y - gy));
	};

	struct Node { int idx; float f; };
	struct Cmp { bool operator()(const Node& a, const Node& b) const { return a.f > b.f; } };
	std::priority_queue<Node, std::vector<Node>, Cmp> open;

	int sFlat = Index1D(sx, sy, W);
	grid[sFlat].g = 0.0f;
	grid[sFlat].f = Heuristic(sx, sy);
	grid[sFlat].opened = true;
	open.push({sFlat, grid[sFlat].f});

	const int kDir[4][2] = {{1,0},{-1,0},{0,1},{0,-1}};
	int goalFound = -1;
	while (!open.empty()) {
		auto cur = open.top();
		open.pop();
		int cIdx = cur.idx;
		if (grid[cIdx].closed) {
			continue;
		}
		grid[cIdx].closed = true;

		if (cIdx == gFlat) {
			goalFound = cIdx;
			break;
		}

		int cx = cIdx % W;
		int cy = cIdx / W;
		for (auto& d : kDir) {
			int nx = cx + d[0];
			int ny = cy + d[1];
			if (nx < 0 || ny < 0 || nx >= W || ny >= H) {
				continue;
			}
			if (!IsTileWalkable(mapChipField_, (uint32_t)nx, (uint32_t)ny)) {
				continue;
			}
			int nIdx = Index1D(nx, ny, W);
			if (grid[nIdx].closed) {
				continue;
			}
			float ng = grid[cIdx].g + 1.0f;
			if (!grid[nIdx].opened || ng < grid[nIdx].g) {
				grid[nIdx].g = ng;
				grid[nIdx].f = ng + Heuristic(nx, ny);
				grid[nIdx].parent = cIdx;
				grid[nIdx].opened = true;
				open.push({nIdx, grid[nIdx].f});
			}
		}
	}

	if (goalFound < 0) {
		return false;
	}

	std::vector<int> rev;
	for (int p = goalFound; p >= 0; p = grid[p].parent) {
		rev.push_back(p);
		if (p == sFlat) {
			break;
		}
	}
	std::reverse(rev.begin(), rev.end());
	outPathFlats = std::move(rev);
	return true;
}

void Enemy::BuildWorldPathFromFlats_(int W, const std::vector<int>& pathFlats) {
	currentPath_.clear();	
	currentPath_.reserve(pathFlats.size());
	for (int flat : pathFlats) {
		int tx = flat % W;
		int ty = flat / W;
		TuboEngine::Math::Vector3 center = mapChipField_->GetMapChipPositionByIndex((uint32_t)tx, (uint32_t)ty);
		center.z = position_.z;
		currentPath_.push_back(center);
	}
	pathCursor_ = 0;
	waypointArriveEps_ = std::max(0.1f, MapChipField::GetBlockSize() * 0.25f);
}

bool Enemy::BuildPathTo(const TuboEngine::Math::Vector3& worldGoal) {
	int W = 0, H = 0;
	int sx = 0, sy = 0, gx = 0, gy = 0;
	if (!PreparePathfinding_(worldGoal, W, H, sx, sy, gx, gy)) {
		return false;
	}

	int gFlat = FindNearestWalkableIndex(mapChipField_, gx, gy);
	if (gFlat < 0) {
		return false;
	}
	gx = gFlat % W;
	gy = gFlat / W;
	lastPathGoalIndex_ = gFlat;

	if (!IsTileWalkable(mapChipField_, (uint32_t)sx, (uint32_t)sy)) {
		int sFlat2 = FindNearestWalkableIndex(mapChipField_, sx, sy);
		if (sFlat2 < 0) {
			return false;
		}
		sx = sFlat2 % W;
		sy = sFlat2 / W;
	}

	std::vector<int> pathFlats;
	if (!RunAStar_(W, H, sx, sy, lastPathGoalIndex_, pathFlats)) {
		return false;
	}

	BuildWorldPathFromFlats_(W, pathFlats);
	return !currentPath_.empty();
}

bool Enemy::UpdateDeathIfNeeded_() {
	if (isAlive_) {
		return false;
	}
	if (!deathEffectPlayed_) {
		EmitDeathParticle();
		deathEffectPlayed_ = true;
	}
	if (deathEmitter_) {
		deathEmitter_->GetPreset().center = position_;
	}
	return true;
}

void Enemy::UpdatePerception_(float dt, bool& outCanSeePlayer, float& outDistanceToPlayer) {
	outDistanceToPlayer = 0.0f;
	if (player_) {
		TuboEngine::Math::Vector3 toPlayer = player_->GetPosition() - position_;
		outDistanceToPlayer = std::sqrt(toPlayer.x * toPlayer.x + toPlayer.y * toPlayer.y + toPlayer.z * toPlayer.z);
	}
	outCanSeePlayer = CanSeePlayer();

	wasJustFound_ = (!sawPlayerPrev_ && outCanSeePlayer);
	wasJustLost_ = (sawPlayerPrev_ && !outCanSeePlayer);
	sawPlayerPrev_ = outCanSeePlayer;

	if (outCanSeePlayer) {
		lastSeenPlayerPos_ = player_->GetPosition();
		lastSeenTimer_ = lastSeenDuration_;
		ClearPath();
		if (wasJustFound_) {
			exclamationTimer_ = iconDuration_;
			questionTimer_ = 0.0f;
		}
	} else if (lastSeenTimer_ > 0.0f) {
		lastSeenTimer_ -= dt;
		if (wasJustLost_) {
			questionTimer_ = iconDuration_;
			exclamationTimer_ = 0.0f;
		}
	}
}

void Enemy::UpdateIcons_(float dt) {
	if (questionTimer_ > 0.0f) {
		questionTimer_ -= dt;
		if (questionTimer_ < 0.0f) questionTimer_ = 0.0f;
	}
	if (exclamationTimer_ > 0.0f) {
		exclamationTimer_ -= dt;
		if (exclamationTimer_ < 0.0f) exclamationTimer_ = 0.0f;
	}
}

void Enemy::UpdateState_(bool canSeePlayer, float distanceToPlayer) {
	if (!player_) {
		return;
	}
	if (canSeePlayer) {
		if (distanceToPlayer > moveStartDistance_) {
			state_ = State::Idle;
		} else if (distanceToPlayer > shootDistance_) {
			state_ = State::Chase;
		} else {
			state_ = State::Attack;
		}
	} else if (lastSeenTimer_ > 0.0f) {
		state_ = State::Alert;
	} else if (state_ == State::Alert) {
		state_ = State::LookAround;
	} else if (state_ != State::LookAround) {
		state_ = State::Idle;
	}
}

void Enemy::UpdateFacing_(bool canSeePlayer) {
	if (!player_) {
		return;
	}
	if (!(state_ == State::Chase || state_ == State::Attack || state_ == State::Alert)) {
		return;
	}
	TuboEngine::Math::Vector3 targetPos = (canSeePlayer) ? player_->GetPosition() : lastSeenPlayerPos_;
	TuboEngine::Math::Vector3 toTarget = targetPos - position_;
	float angleZ = std::atan2(toTarget.y, toTarget.x);
	float diff = NormalizeAngle(angleZ - rotation_.z);
	float maxTurn = turnSpeed_;
	if (std::fabs(diff) < maxTurn) {
		rotation_.z = angleZ;
	} else {
		rotation_.z += (diff > 0 ? 1 : -1) * maxTurn;
		rotation_.z = NormalizeAngle(rotation_.z);
	}
}

void Enemy::UpdateShooting_(float dt, bool canSeePlayer) {
	wantShoot_ = (canSeePlayer && (state_ == State::Chase || state_ == State::Attack));
	if (wantShoot_) {
		bulletTimer_ += dt;
	} else {
		bulletTimer_ = std::min(bulletTimer_, EnemyNormalBullet::s_fireInterval);
	}

	auto TryFire = [this]() {
		if (!wantShoot_) return;
		if (bullet_ && bullet_->GetIsAlive()) return;
		if (bullet_ && !bullet_->GetIsAlive()) bullet_.reset();
		if (bulletTimer_ < EnemyNormalBullet::s_fireInterval) return;
		bulletTimer_ = 0.0f;
		bullet_ = std::make_unique<EnemyNormalBullet>();
		bullet_->Initialize(position_);
		bullet_->SetEnemyPosition(position_);
		bullet_->SetEnemyRotation(rotation_);
		bullet_->SetPlayer(player_);
		bullet_->SetCamera(camera_);
		bullet_->SetMapChipField(mapChipField_);
	};

	// 状態ごとの行動で必要な箇所のみ呼ぶため、ここではラムダを保持できない。
	// 代わりに wantShoot_/bulletTimer_ はここで整えて、各State側で発射判定する。
	// ※既存挙動: Chase/AttackでのみTryFireが走る
	if (state_ == State::Chase || state_ == State::Attack) {
		TryFire();
	}
}

void Enemy::UpdateBehaviorByState_(float dt, bool /*canSeePlayer*/) {
	switch (state_) {
	case State::Idle: {
		switch (idleBackPhase_) {
		case IdleBackPhase::None:
			idleLookAroundTimer_ -= dt;
			if (idleLookAroundTimer_ <= 0.0f) {
				idleBackStartAngle_ = rotation_.z;
				idleBackTargetAngle_ = NormalizeAngle(idleBackStartAngle_ + kPI);
				idleBackHoldTimer_ = idleBackHoldSec_;
				idleBackPhase_ = IdleBackPhase::ToBack;
			}
			break;
		case IdleBackPhase::ToBack: {
			float diff = NormalizeAngle(idleBackTargetAngle_ - rotation_.z);
			float turn = std::clamp(diff, -idleBackTurnSpeed_, idleBackTurnSpeed_);
			rotation_.z = NormalizeAngle(rotation_.z + turn);
			if (std::fabs(diff) < 0.01f) {
				rotation_.z = idleBackTargetAngle_;
				idleBackPhase_ = IdleBackPhase::Hold;
			}
		} break;
		case IdleBackPhase::Hold:
			idleBackHoldTimer_ -= dt;
			if (idleBackHoldTimer_ <= 0.0f) idleBackPhase_ = IdleBackPhase::Return;
			break;
		case IdleBackPhase::Return: {
			float diff = NormalizeAngle(idleBackStartAngle_ - rotation_.z);
			float turn = std::clamp(diff, -idleBackTurnSpeed_, idleBackTurnSpeed_);
			rotation_.z = NormalizeAngle(rotation_.z + turn);
			if (std::fabs(diff) < 0.01f) {
				rotation_.z = idleBackStartAngle_;
				idleBackPhase_ = IdleBackPhase::None;
				idleLookAroundTimer_ = idleLookAroundIntervalSec_;
			}
		} break;
		}
		break;
	}
	case State::Alert:
		idleBackPhase_ = IdleBackPhase::None;
		break;
	case State::LookAround: {
		idleBackPhase_ = IdleBackPhase::None;
		if (!lookAroundInitialized_) {
			lookAroundBaseAngle_ = rotation_.z;
			lookAroundTargetAngle_ = lookAroundBaseAngle_ + lookAroundAngleWidth_;
			lookAroundDirection_ = 1;
			lookAroundCount_ = 0;
			lookAroundInitialized_ = true;
		}
		float diff = NormalizeAngle(lookAroundTargetAngle_ - rotation_.z);
		float turn = std::clamp(diff, -lookAroundSpeed_, lookAroundSpeed_);
		rotation_.z = NormalizeAngle(rotation_.z + turn);
		if (std::fabs(diff) < 0.02f) {
			lookAroundDirection_ *= -1;
			lookAroundTargetAngle_ = lookAroundBaseAngle_ + lookAroundDirection_ * lookAroundAngleWidth_;
			lookAroundCount_++;
			if (lookAroundCount_ >= lookAroundMaxCount_) {
				lookAroundInitialized_ = false;
				state_ = State::Patrol;
			}
		}
		break;
	}
	case State::Patrol:
		idleBackPhase_ = IdleBackPhase::None;
		state_ = State::Idle;
		break;
	case State::Chase: {
		idleBackPhase_ = IdleBackPhase::None;
		if (player_) {
			TuboEngine::Math::Vector3 dir = player_->GetPosition() - position_;
			dir.z = 0.0f;
			float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
			if (len > 0.1f) {
				dir.x /= len;
				dir.y /= len;
				TuboEngine::Math::Vector3 desiredMove{dir.x * moveSpeed_, dir.y * moveSpeed_, 0.0f};
				MoveWithCollision(position_, desiredMove, mapChipField_);
			}
		}
		break;
	}
	case State::Attack:
		idleBackPhase_ = IdleBackPhase::None;
		break;
	}
}

void Enemy::UpdateBullet_() {
	if (bullet_ && bullet_->GetIsAlive()) {
		bullet_->Update();
	}
}

void Enemy::SyncToObject3d_() {
	object3d_->SetPosition(position_);
	TuboEngine::Math::Vector3 drawRot = rotation_;
	drawRot.x = NormalizeAngle(drawRot.x + kEnemyModelRotOffsetX);
	drawRot.y = NormalizeAngle(drawRot.y + kEnemyModelRotOffsetY);
	drawRot.z = NormalizeAngle(drawRot.z + kEnemyModelRotOffsetZ);
	object3d_->SetRotation(drawRot);
	object3d_->SetScale(scale_);
	object3d_->SetCamera(camera_);
	object3d_->Update();
}

void Enemy::SyncEmitters_() {
	if (hitEmitter_) hitEmitter_->GetPreset().center = position_;
	if (deathEmitter_ && !deathEffectPlayed_) deathEmitter_->GetPreset().center = position_;
}

void Enemy::UpdateHitAndResetFlags_() {
	if (!wasHit_ && isHit_) {
		EmitHitParticle();
	}
	wasHit_ = isHit_;
	isHit_ = false;
}

void Enemy::UpdateIconSprites_() {
	TuboEngine::Math::Vector3 iconWorldPos = position_;
	iconWorldPos.z = position_.z;
	iconWorldPos.y += iconOffsetY_;
	if (questionTimer_ > 0.0f && questionIcon_) {
		questionIcon_->SetPosition({iconWorldPos.x, iconWorldPos.y});
		questionIcon_->SetSize(iconSize_);
		questionIcon_->Update();
	}
	if (exclamationTimer_ > 0.0f && exclamationIcon_) {
		exclamationIcon_->SetPosition({iconWorldPos.x, iconWorldPos.y});
		exclamationIcon_->SetSize(iconSize_);
		exclamationIcon_->Update();
	}
}

void Enemy::Update() {
	if (UpdateDeathIfNeeded_()) {
		return;
	}

	const float dt = GetFixedDeltaTime_();
	ApplyKnockback(dt);

	float distanceToPlayer = 0.0f;
	bool canSeePlayer = false;
	UpdatePerception_(dt, canSeePlayer, distanceToPlayer);
	UpdateIcons_(dt);
	UpdateState_(canSeePlayer, distanceToPlayer);
	UpdateFacing_(canSeePlayer);
	UpdateShooting_(dt, canSeePlayer);
	UpdateBehaviorByState_(dt, canSeePlayer);
	UpdateBullet_();
	SyncToObject3d_();
	SyncEmitters_();
	UpdateHitAndResetFlags_();
	UpdateIconSprites_();
}

void Enemy::ApplyKnockback(float dt) {
    if (knockbackTimer_ > 0.0f) {
		TuboEngine::Math::Vector3 kbStep = {knockbackVelocity_.x * dt, knockbackVelocity_.y * dt, 0.0f};
        MoveWithCollision(position_, kbStep, mapChipField_);
        knockbackVelocity_.x *= knockbackDamping_;
        knockbackVelocity_.y *= knockbackDamping_;
        knockbackTimer_ -= dt;
        if (knockbackTimer_ <= 0.0f) {
            knockbackTimer_ = 0.0f;
            knockbackVelocity_ = {0.0f, 0.0f, 0.0f};
        }
    }
}

void Enemy::EmitHitParticle() {
    if (hitEmitter_) {
        hitEmitter_->GetPreset().center = position_;
        hitEmitter_->Emit(hitEmitter_->GetPreset().burstCount);
    }
    if (hitRingEmitter_) {
        hitRingEmitter_->GetPreset().center = position_;
        hitRingEmitter_->Emit(hitRingEmitter_->GetPreset().burstCount);
    }
}

void Enemy::EmitDeathParticle() {
    if (!deathEmitter_)
        return;
    deathEmitter_->Emit(deathEmitter_->GetPreset().burstCount);
}

void Enemy::OnCollision(Collider* other) {
    if (!other) return;
    const uint32_t typeID = other->GetTypeID();
    if (typeID != static_cast<uint32_t>(CollisionTypeId::kPlayerWeapon)) return;
    isHit_ = true;
    hp_ -= PlayerBullet::s_damage;

    // 視認してないときに攻撃されたら Question と Exclamation を同時に表示
    if (!sawPlayerPrev_) {
        questionTimer_ = iconDuration_;
        exclamationTimer_ = iconDuration_;
    }

    // ノックバック（弾位置基準）
	TuboEngine::Math::Vector3 hitPos;
    if (auto* bullet = dynamic_cast<PlayerBullet*>(other)) hitPos = bullet->GetCenterPosition();
    else if (player_) hitPos = player_->GetPosition();
    else hitPos = position_;
	TuboEngine::Math::Vector3 away = position_ - hitPos;
	away.z = 0.0f;
    float len = std::sqrt(away.x * away.x + away.y * away.y);
    if (len > 0.001f) { away.x /= len; away.y /= len; }
    const float tile = MapChipField::GetBlockSize();
    float speed = tile * knockbackStrength_;
    knockbackVelocity_.x = away.x * speed;
    knockbackVelocity_.y = away.y * speed;
    knockbackTimer_ = 0.12f;

    if (hp_ <= 0 && isAlive_) {
        isAlive_ = false;
        EmitDeathParticle();
        deathEffectPlayed_ = true;
    }
    if (state_ == State::Idle || state_ == State::Alert) {
        if (player_) { lastSeenPlayerPos_ = player_->GetPosition(); lastSeenTimer_ = lastSeenDuration_; }
        state_ = State::Chase;
    }
}

void Enemy::Draw() {
    if (isAlive_ == false) { /* 死亡後は通常モデル非表示 */
        return;
    }
    if (object3d_)
        object3d_->Draw();
    if (bullet_)
        bullet_->Draw();
    DrawViewCone();
    DrawLastSeenMark();
    DrawStateIcon();
    // 2Dスプライトはここでは描かない（SpriteDrawフェーズで個別に描画）
}
void Enemy::ParticleDraw() { /* legacy */ }

void Enemy::DrawImGui() {
#ifdef USE_IMGUI
    ImGui::Begin("Enemy");
    ImGui::Text("Pos:(%.2f,%.2f,%.2f)", position_.x, position_.y, position_.z);
    ImGui::Text("HP:%d", hp_);
    ImGui::Text("Alive:%s", isAlive_ ? "Yes" : "No");
    ImGui::Text("State:%d", (int)state_);
    if (hitEmitter_) { auto& p = hitEmitter_->GetPreset(); ImGui::Text("HitEmitter rate:%.1f", p.emitRate); }
    if (deathEmitter_) { auto& p = deathEmitter_->GetPreset(); ImGui::Text("DeathEmitter life:[%.2f,%.2f]", p.lifeMin, p.lifeMax); }
    ImGui::Separator();
    ImGui::Text("KnockbackTimer: %.2f", knockbackTimer_);
    ImGui::DragFloat("KnockbackStrength", &knockbackStrength_, 0.01f, 0.1f, 10.0f);
    ImGui::DragFloat("KnockbackDamping", &knockbackDamping_, 0.01f, 0.5f, 0.99f);
    ImGui::Separator();
    // 視野角・視認距離を調整
    float angle = GetViewAngleDeg();
    if (ImGui::DragFloat("View Angle (deg)", &angle, 0.5f, 1.0f, 360.0f)) {
        SetViewAngleDeg(angle);
    }
    float dist = GetViewDistance();
    if (ImGui::DragFloat("View Distance", &dist, 0.1f, 0.0f, 200.0f)) {
        SetViewDistance(dist);
    }
    ImGui::Checkbox("Show Ray Samples", &showRaySamples_);
    ImGui::DragFloat("Icon Duration", &iconDuration_, 0.01f, 0.2f, 3.0f);
    ImGui::DragFloat("Icon Offset Y", &iconOffsetY_, 0.01f, 0.0f, 5.0f);
    ImGui::DragFloat2("Icon Size", &iconSize_.x, 0.5f, 16.0f, 128.0f);
    ImGui::End();
#endif
}

bool Enemy::CanSeePlayer() {
    if (!player_ || !mapChipField_)
        return false;

    TuboEngine::Math::Vector3 from = position_;
	TuboEngine::Math::Vector3 to = player_->GetPosition();
	TuboEngine::Math::Vector3 dirToPlayer = to - from;
    dirToPlayer.z = 0.0f;

    // 距離判定
    float distance = std::sqrt(dirToPlayer.x * dirToPlayer.x + dirToPlayer.y * dirToPlayer.y);
    if (distance > viewDistance_)
        return false;

    // 視野角判定
    // ここは「モデル見た目」ではなく「ロジック上の前方」に合わせるため、
    // 描画補正オフセットは適用しない。
	TuboEngine::Math::Vector3 forward = {std::cos(rotation_.z), std::sin(rotation_.z), 0.0f};
	float dot = TuboEngine::Math::Vector3::Dot(TuboEngine::Math::Vector3::Normalize(forward), TuboEngine::Math::Vector3::Normalize(dirToPlayer));
    dot = std::clamp(dot, -1.0f, 1.0f); // acosの安全化
    float angleToPlayer = std::acos(dot) * 180.0f / kPI;
    if (angleToPlayer > viewAngleDeg_ / 2.0f)
        return false;

    // レイ遮蔽物判定 (簡易サンプリング)
	TuboEngine::Math::Vector2 start = {from.x, from.y};
	TuboEngine::Math::Vector2 end = {to.x, to.y};
    const float step = 0.5f;
	TuboEngine::Math::Vector2 dir = end - start;
    float length = std::sqrt(dir.x * dir.x + dir.y * dir.y);
    if (length < 0.001f)
        return true;
    dir.x /= length;
    dir.y /= length;
    for (float t = 0.0f; t <= length; t += step) {
		TuboEngine::Math::Vector2 pos2 = start + dir * t;
		TuboEngine::Math::Vector3 checkPos = {pos2.x, pos2.y, from.z};
        if (mapChipField_->IsBlocked(checkPos)) {
            return false; // 壁などで遮られている
        }
    }
    return true;
}

void Enemy::DrawViewCone() {
    float halfRad = (viewAngleDeg_ / 2.0f) * kPI / 180.0f;
    float baseAngle = rotation_.z;
	TuboEngine::Math::Vector3 center = position_;

    Vector4 colNone = {1.0f, 1.0f, 0.0f, 0.7f};
    Vector4 colAware = {1.0f, 0.6f, 0.0f, 0.8f};
    Vector4 colFound = {1.0f, 0.2f, 0.2f, 0.9f};
    bool canSee = CanSeePlayer();
    Vector4 coneColor = canSee ? colFound : (lastSeenTimer_ > 0.0f ? colAware : colNone);

    for (int i = 0; i < viewLineDiv_; ++i) {
        float a0 = baseAngle - halfRad + (halfRad * 2.0f) * (float(i) / viewLineDiv_);
		TuboEngine::Math::Vector3 p0 = center + TuboEngine::Math::Vector3{std::cos(a0) * viewDistance_, std::sin(a0) * viewDistance_, 0.0f};
        p0.z = center.z;
        LineManager::GetInstance()->DrawLine(center, p0, coneColor);
    }
    for (int i = 0; i < viewLineDiv_ - 1; ++i) {
        float a0 = baseAngle - halfRad + (halfRad * 2.0f) * (float(i) / viewLineDiv_);
        float a1 = baseAngle - halfRad + (halfRad * 2.0f) * (float(i + 1) / viewLineDiv_);
		TuboEngine::Math::Vector3 p0 = center + TuboEngine::Math::Vector3{std::cos(a0) * viewDistance_, std::sin(a0) * viewDistance_, 0.0f};
		TuboEngine::Math::Vector3 p1 = center + TuboEngine::Math::Vector3{std::cos(a1) * viewDistance_, std::sin(a1) * viewDistance_, 0.0f};
        p0.z = p1.z = center.z;
        LineManager::GetInstance()->DrawLine(p0, p1, coneColor);
    }

    // レイサンプルはデバッグフラグがONかつプレイヤーが存在する時のみ描画
    if (showRaySamples_ && player_ && mapChipField_) {
		TuboEngine::Math::Vector2 start = {center.x, center.y};
		TuboEngine::Math::Vector2 end = {player_->GetPosition().x, player_->GetPosition().y};
        const float step = 0.5f;
		TuboEngine::Math::Vector2 dir = end - start;
        float length = std::sqrt(dir.x * dir.x + dir.y * dir.y);
        if (length > 0.001f) {
            dir.x /= length; dir.y /= length;
            for (float t = 0.0f; t <= length; t += step) {
				TuboEngine::Math::Vector2 pos2 = start + dir * t;
				TuboEngine::Math::Vector3 checkPos = {pos2.x, pos2.y, center.z};
                bool blocked = mapChipField_->IsBlocked(checkPos);
                Vector4 dotCol = blocked ? Vector4{1.0f, 0.1f, 0.1f, 1.0f} : Vector4{0.2f, 1.0f, 0.2f, 1.0f};
				TuboEngine::Math::Vector3 pA = {checkPos.x - 0.05f, checkPos.y - 0.05f, checkPos.z};
				TuboEngine::Math::Vector3 pB = {checkPos.x + 0.05f, checkPos.y + 0.05f, checkPos.z};
                LineManager::GetInstance()->DrawLine(pA, pB, dotCol);
            }
        }
    }
}

void Enemy::DrawLastSeenMark() {
    if (lastSeenTimer_ <= 0.0f)
        return;
    constexpr Vector4 kLastSeenColor = {1.0f, 0.2f, 0.2f, 1.0f};
    constexpr float kLastSeenMarkSize = 0.5f;
	const TuboEngine::Math::Vector3& center = lastSeenPlayerPos_;
    // 十字
	LineManager::GetInstance()->DrawLine(center + TuboEngine::Math::Vector3{-kLastSeenMarkSize, 0.0f, 0.0f}, center + TuboEngine::Math::Vector3{kLastSeenMarkSize, 0.0f, 0.0f}, kLastSeenColor);
	LineManager::GetInstance()->DrawLine(center + TuboEngine::Math::Vector3{0.0f, -kLastSeenMarkSize, 0.0f}, center + TuboEngine::Math::Vector3{0.0f, kLastSeenMarkSize, 0.0f}, kLastSeenColor);
    // 外周円
    constexpr int circleDiv = 16;
    for (int i = 0; i < circleDiv; ++i) {
        float a0 = (2.0f * kPI) * (float(i) / circleDiv);
        float a1 = (2.0f * kPI) * (float(i + 1) / circleDiv);
		TuboEngine::Math::Vector3 p0 = center + TuboEngine::Math::Vector3{std::cos(a0) * kLastSeenMarkSize, std::sin(a0) * kLastSeenMarkSize, 0.0f};
		TuboEngine::Math::Vector3 p1 = center + TuboEngine::Math::Vector3{std::cos(a1) * kLastSeenMarkSize, std::sin(a1) * kLastSeenMarkSize, 0.0f};
        LineManager::GetInstance()->DrawLine(p0, p1, kLastSeenColor);
    }
}

// 末尾に欠けていた関数を追加
TuboEngine::Math::Vector3 Enemy::GetCenterPosition() const {
    // 当たり判定中心（今はスケール考慮せずそのまま）    
    return position_;
}

void Enemy::DrawStateIcon() {
    // 状態に応じた簡易アイコンラインを頭上に描画
    if (!isAlive_)
        return;
	TuboEngine::Math::Vector3 base = position_;
    base.z += 0.0f; // 2D平面なのでそのまま
	TuboEngine::Math::Vector3 upPos = base + TuboEngine::Math::Vector3{0, 0, 0};
    (void)upPos;

    Vector4 col;
    switch (state_) {
    case State::Idle:
        col = {0.4f, 0.4f, 0.4f, 0.8f};
        break;
    case State::Alert:
        col = {1.0f, 0.8f, 0.2f, 0.9f};
        break;
    case State::LookAround:
        col = {0.2f, 0.7f, 1.0f, 0.9f};
        break;
    case State::Patrol:
        col = {0.3f, 0.9f, 0.3f, 0.9f};
        break;
    case State::Chase:
        col = {1.0f, 0.2f, 0.2f, 0.9f};
        break;
    case State::Attack:
        col = {1.0f, 0.0f, 1.0f, 0.9f};
        break;
    }
    float s = 0.4f;
    // 菱形
	TuboEngine::Math::Vector3 p0 = base + TuboEngine::Math::Vector3{0, -s, 0};
	TuboEngine::Math::Vector3 p1 = base + TuboEngine::Math::Vector3{s, 0, 0};
	TuboEngine::Math::Vector3 p2 = base + TuboEngine::Math::Vector3{0, s, 0};
	TuboEngine::Math::Vector3 p3 = base + TuboEngine::Math::Vector3{-s, 0, 0};
    LineManager::GetInstance()->DrawLine(p0, p1, col);
    LineManager::GetInstance()->DrawLine(p1, p2, col);
    LineManager::GetInstance()->DrawLine(p2, p3, col);
    LineManager::GetInstance()->DrawLine(p3, p0, col);
}