#include "Enemy.h"
#include "Bullet/Player/PlayerBullet.h"
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
#include "engine/graphic/Particle/ParticleManager.h"
#include "engine/graphic/Particle/PrimitiveEmitter.h"
#include "engine/graphic/Particle/RingEmitter.h"

constexpr float kPI = 3.14159265358979323846f;

Enemy::Enemy() {}
Enemy::~Enemy() {}

void Enemy::Initialize() {
    Collider::SetTypeID(static_cast<uint32_t>(CollisionTypeId::kEnemy));
    position = {0.0f, 0.0f, 5.0f};
    rotation = {};
    scale = {1.0f, 1.0f, 1.0f};

    object3d = std::make_unique<Object3d>();
    const std::string modelFileNamePath = "player/player.obj";
    object3d->Initialize(modelFileNamePath);
    object3d->SetPosition(position);
    object3d->SetRotation(rotation);
    object3d->SetScale(scale);
	object3d->SetModelColor({1.0f, 0.5f, 0.5f, 1.0f});

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

    HP = 10; // 調整: 少し耐える

    idleLookAroundTimer = idleLookAroundIntervalSec;
    idleBackPhase_ = IdleBackPhase::None;
    idleBackHoldTimer = 0.0f;
    idleBackStartAngle = rotation.z;
    idleBackTargetAngle = rotation.z;

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
        p.center = position;
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
        p.center = position;
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
        p.center = position;
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

static void MoveWithCollision(Vector3& position, const Vector3& desiredMove, MapChipField* field) {
    if (!field) {
        position = position + desiredMove;
        return;
    }
    const float tile = MapChipField::GetBlockSize();
    const float width = tile * 0.8f;
    const float height = tile * 0.8f;
    float moveLen2D = std::sqrt(desiredMove.x * desiredMove.x + desiredMove.y * desiredMove.y);
    int subSteps = std::max(1, int(std::ceil(moveLen2D / (tile * 0.5f))));
    Vector3 step = desiredMove / float(subSteps);
    for (int i = 0; i < subSteps; ++i) {
        Vector3 nextX = position;
        nextX.x += step.x;
        if (!field->IsRectBlocked(nextX, width, height)) {
            position = nextX;
        }
        Vector3 nextY = position;
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

            float ng = grid[cIdx].g + 1.0f; // 4近傑=コスト1
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
    // 死亡演出再生後は描画のみ (Update 最低限)
    if (!isAllive) {
        if (!deathEffectPlayed_) {
            EmitDeathParticle();
            deathEffectPlayed_ = true;
        }
        if (deathEmitter_) {
            deathEmitter_->GetPreset().center = position;
        }
        return;
    }

    const float dt = 1.0f / 60.0f;

    // --- ノックバック適用（先に移動へ反映）---
    ApplyKnockback(dt);

    float distanceToPlayer = 0.0f;
    Vector3 toPlayer = {0, 0, 0};
    if (player_) {
        toPlayer = player_->GetPosition() - position;
        distanceToPlayer = std::sqrt(toPlayer.x * toPlayer.x + toPlayer.y * toPlayer.y + toPlayer.z * toPlayer.z);
    }
    bool canSeePlayer = CanSeePlayer();

    // 視認状態フラグ更新
    wasJustFound_ = (!sawPlayerPrev_ && canSeePlayer);
    wasJustLost_  = (sawPlayerPrev_ && !canSeePlayer);
    sawPlayerPrev_ = canSeePlayer;

    if (canSeePlayer) {
        lastSeenPlayerPos = player_->GetPosition();
        lastSeenTimer = kLastSeenDuration;
        ClearPath();
        // 見つけた瞬間にExclamationを表示
        if (wasJustFound_) {
            exclamationTimer_ = iconDuration_;
            questionTimer_ = 0.0f;
        }
    } else if (lastSeenTimer > 0.0f) {
        lastSeenTimer -= dt;
        // 見失った瞬間にQuestionを表示
        if (wasJustLost_) {
            questionTimer_ = iconDuration_;
            exclamationTimer_ = 0.0f;
        }
    }

    // アイコンタイマー更新
    if (questionTimer_ > 0.0f) { questionTimer_ -= dt; if (questionTimer_ < 0.0f) questionTimer_ = 0.0f; }
    if (exclamationTimer_ > 0.0f) { exclamationTimer_ -= dt; if (exclamationTimer_ < 0.0f) exclamationTimer_ = 0.0f; }

    if (player_) {
        if (canSeePlayer) {
            if (distanceToPlayer > moveStartDistance_)
                state_ = State::Idle;
            else if (distanceToPlayer > shootDistance_)
                state_ = State::Chase;
            else
                state_ = State::Attack;
        } else if (lastSeenTimer > 0.0f)
            state_ = State::Alert;
        else if (state_ == State::Alert)
            state_ = State::LookAround;
        else if (state_ != State::LookAround)
            state_ = State::Idle;
    }

    // プレイヤーの方向を向く（一定速度で回転）
    if (player_ && (state_ == State::Chase || state_ == State::Attack || state_ == State::Alert)) {
        Vector3 targetPos = (canSeePlayer) ? player_->GetPosition() : lastSeenPlayerPos;
        Vector3 toTarget = targetPos - position;
        float angleZ = std::atan2(toTarget.y, toTarget.x);
        float diff = NormalizeAngle(angleZ - rotation.z);
        float maxTurn = turnSpeed_;
        if (std::fabs(diff) < maxTurn)
            rotation.z = angleZ;
        else {
            rotation.z += (diff > 0 ? 1 : -1) * maxTurn;
            rotation.z = NormalizeAngle(rotation.z);
        }
    }

    // --- 射撃条件評価 ---
    wantShoot_ = (canSeePlayer && (state_ == State::Chase || state_ == State::Attack));

    // クールダウンタイマー更新
    if (wantShoot_)
        bulletTimer_ += dt;
    else
        bulletTimer_ = std::min(bulletTimer_, EnemyNormalBullet::s_fireInterval);
    auto TryFire = [this]() {
        if (!wantShoot_)
            return;
        if (bullet && bullet->GetIsAlive())
            return;
        if (bullet && !bullet->GetIsAlive())
            bullet.reset();
        if (bulletTimer_ < EnemyNormalBullet::s_fireInterval)
            return;
        bulletTimer_ = 0.0f;
        bullet = std::make_unique<EnemyNormalBullet>();
        bullet->Initialize(position);
        bullet->SetEnemyPosition(position);
        bullet->SetEnemyRotation(rotation);
        bullet->SetPlayer(player_);
        bullet->SetCamera(camera_);
        bullet->SetMapChipField(mapChipField);
    };

    // 状態ごとの行動（ノックバック中も自律移動は行うが、速度が小さくなる）
    switch (state_) {
    case State::Idle: {
        switch (idleBackPhase_) {
        case IdleBackPhase::None:
            idleLookAroundTimer -= dt;
            if (idleLookAroundTimer <= 0.0f) {
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
            if (idleBackHoldTimer <= 0.0f)
                idleBackPhase_ = IdleBackPhase::Return;
            break;
        case IdleBackPhase::Return: {
            float diff = NormalizeAngle(idleBackStartAngle - rotation.z);
            float turn = std::clamp(diff, -idleBackTurnSpeed, idleBackTurnSpeed);
            rotation.z = NormalizeAngle(rotation.z + turn);
            if (std::fabs(diff) < 0.01f) {
                rotation.z = idleBackStartAngle;
                idleBackPhase_ = IdleBackPhase::None;
                idleLookAroundTimer = idleLookAroundIntervalSec;
            }
        } break;
        }
        break;
    }
    case State::Alert: {
        idleBackPhase_ = IdleBackPhase::None;
        break;
    }
    case State::LookAround: {
        idleBackPhase_ = IdleBackPhase::None;
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
        idleBackPhase_ = IdleBackPhase::None;
        state_ = State::Idle;
        break;
    }
    case State::Chase: {
        idleBackPhase_ = IdleBackPhase::None;
        if (player_) {
            Vector3 dir = player_->GetPosition() - position;
            dir.z = 0.0f;
            float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
            if (len > 0.1f) {
                dir.x /= len;
                dir.y /= len;
                Vector3 desiredMove{dir.x * moveSpeed_, dir.y * moveSpeed_, 0.0f};
                MoveWithCollision(position, desiredMove, mapChipField);
            }
        }
        TryFire();
        break;
    }
    case State::Attack: {
        idleBackPhase_ = IdleBackPhase::None;
        TryFire();
        break;
    }
    }

    if (bullet && bullet->GetIsAlive())
        bullet->Update();

    object3d->SetPosition(position);
    object3d->SetRotation(rotation);
    object3d->SetScale(scale);
    object3d->SetCamera(camera_);
    object3d->Update();

    // エミッタ中心追随
    if (hitEmitter_)
        hitEmitter_->GetPreset().center = position;
    if (deathEmitter_ && !deathEffectPlayed_)
        deathEmitter_->GetPreset().center = position;

    if (!wasHit && isHit) {
        EmitHitParticle();
    }
    wasHit = isHit;
    isHit = false;

    // アイコン表示更新
    Vector3 iconWorldPos = position; iconWorldPos.z = position.z;
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

void Enemy::ApplyKnockback(float dt) {
    if (knockbackTimer_ > 0.0f) {
        Vector3 kbStep = {knockbackVelocity_.x * dt, knockbackVelocity_.y * dt, 0.0f};
        MoveWithCollision(position, kbStep, mapChipField);
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
    // 既存のヒットスパーク
    if (hitEmitter_) {
        hitEmitter_->GetPreset().center = position;
        hitEmitter_->Emit(hitEmitter_->GetPreset().burstCount);
    }
    // 追加の小リング（既存と併用）
    if (hitRingEmitter_) {
        hitRingEmitter_->GetPreset().center = position;
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
    isHit = true;
    HP -= PlayerBullet::s_damage;

    // 視認してないときに攻撃されたら Question と Exclamation を同時に表示
    if (!sawPlayerPrev_) {
        questionTimer_ = iconDuration_;
        exclamationTimer_ = iconDuration_;
    }

    // ノックバック（弾位置基準）
    Vector3 hitPos;
    if (auto* bullet = dynamic_cast<PlayerBullet*>(other)) hitPos = bullet->GetCenterPosition();
    else if (player_) hitPos = player_->GetPosition();
    else hitPos = position;
    Vector3 away = position - hitPos; away.z = 0.0f;
    float len = std::sqrt(away.x * away.x + away.y * away.y);
    if (len > 0.001f) { away.x /= len; away.y /= len; }
    const float tile = MapChipField::GetBlockSize();
    float speed = tile * knockbackStrength_;
    knockbackVelocity_.x = away.x * speed;
    knockbackVelocity_.y = away.y * speed;
    knockbackTimer_ = 0.12f;

    if (HP <= 0 && isAllive) {
        isAllive = false;
        EmitDeathParticle();
        deathEffectPlayed_ = true;
    }
    if (state_ == State::Idle || state_ == State::Alert) {
        if (player_) { lastSeenPlayerPos = player_->GetPosition(); lastSeenTimer = kLastSeenDuration; }
        state_ = State::Chase;
    }
}

void Enemy::Draw() {
    if (isAllive == false) { /* 死亡後は通常モデル非表示 */
        return;
    }
    if (object3d)
        object3d->Draw();
    if (bullet)
        bullet->Draw();
    DrawViewCone();
    DrawLastSeenMark();
    DrawStateIcon();
    // 2Dスプライトはここでは描かない（SpriteDrawフェーズで個別に描画）
}
void Enemy::ParticleDraw() { /* legacy */ }

void Enemy::DrawImGui() {
#ifdef USE_IMGUI
    ImGui::Begin("Enemy");
    ImGui::Text("Pos:(%.2f,%.2f,%.2f)", position.x, position.y, position.z);
    ImGui::Text("HP:%d", HP);
    ImGui::Text("Alive:%s", isAllive ? "Yes" : "No");
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
    if (!player_ || !mapChipField)
        return false;

    Vector3 from = position;
    Vector3 to = player_->GetPosition();
    Vector3 dirToPlayer = to - from;
    dirToPlayer.z = 0.0f;

    // 距離判定
    float distance = std::sqrt(dirToPlayer.x * dirToPlayer.x + dirToPlayer.y * dirToPlayer.y);
    if (distance > kViewDistance)
        return false;

    // 視野角判定 (Z回転を前方ベクトルに)
    Vector3 forward = {std::cos(rotation.z), std::sin(rotation.z), 0.0f};
    float dot = Vector3::Dot(Vector3::Normalize(forward), Vector3::Normalize(dirToPlayer));
    float angleToPlayer = std::acos(dot) * 180.0f / kPI;
    if (angleToPlayer > kViewAngleDeg / 2.0f)
        return false;

    // レイ遮蔽物判定 (簡易サンプリング)
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
        Vector2 pos2 = start + dir * t;
        Vector3 checkPos = {pos2.x, pos2.y, from.z};
        if (mapChipField->IsBlocked(checkPos)) {
            return false; // 壁などで遮られている
        }
    }
    return true;
}

void Enemy::DrawViewCone() {
    float halfRad = (kViewAngleDeg / 2.0f) * kPI / 180.0f;
    float baseAngle = rotation.z;
    Vector3 center = position;

    Vector4 colNone = {1.0f, 1.0f, 0.0f, 0.7f};
    Vector4 colAware = {1.0f, 0.6f, 0.0f, 0.8f};
    Vector4 colFound = {1.0f, 0.2f, 0.2f, 0.9f};
    bool canSee = CanSeePlayer();
    Vector4 coneColor = canSee ? colFound : (lastSeenTimer > 0.0f ? colAware : colNone);

    for (int i = 0; i < kViewLineDiv; ++i) {
        float a0 = baseAngle - halfRad + (halfRad * 2.0f) * (float(i) / kViewLineDiv);
        Vector3 p0 = center + Vector3{std::cos(a0) * kViewDistance, std::sin(a0) * kViewDistance, 0.0f};
        p0.z = center.z;
        LineManager::GetInstance()->DrawLine(center, p0, coneColor);
    }
    for (int i = 0; i < kViewLineDiv - 1; ++i) {
        float a0 = baseAngle - halfRad + (halfRad * 2.0f) * (float(i) / kViewLineDiv);
        float a1 = baseAngle - halfRad + (halfRad * 2.0f) * (float(i + 1) / kViewLineDiv);
        Vector3 p0 = center + Vector3{std::cos(a0) * kViewDistance, std::sin(a0) * kViewDistance, 0.0f};
        Vector3 p1 = center + Vector3{std::cos(a1) * kViewDistance, std::sin(a1) * kViewDistance, 0.0f};
        p0.z = p1.z = center.z;
        LineManager::GetInstance()->DrawLine(p0, p1, coneColor);
    }

    // レイサンプルはデバッグフラグがONかつプレイヤーが存在する時のみ描画
    if (showRaySamples_ && player_ && mapChipField) {
        Vector2 start = {center.x, center.y};
        Vector2 end = {player_->GetPosition().x, player_->GetPosition().y};
        const float step = 0.5f;
        Vector2 dir = end - start;
        float length = std::sqrt(dir.x * dir.x + dir.y * dir.y);
        if (length > 0.001f) {
            dir.x /= length; dir.y /= length;
            for (float t = 0.0f; t <= length; t += step) {
                Vector2 pos2 = start + dir * t;
                Vector3 checkPos = {pos2.x, pos2.y, center.z};
                bool blocked = mapChipField->IsBlocked(checkPos);
                Vector4 dotCol = blocked ? Vector4{1.0f, 0.1f, 0.1f, 1.0f} : Vector4{0.2f, 1.0f, 0.2f, 1.0f};
                Vector3 pA = {checkPos.x - 0.05f, checkPos.y - 0.05f, checkPos.z};
                Vector3 pB = {checkPos.x + 0.05f, checkPos.y + 0.05f, checkPos.z};
                LineManager::GetInstance()->DrawLine(pA, pB, dotCol);
            }
        }
    }
}

void Enemy::DrawLastSeenMark() {
    if (lastSeenTimer <= 0.0f)
        return;
    constexpr Vector4 kLastSeenColor = {1.0f, 0.2f, 0.2f, 1.0f};
    constexpr float kLastSeenMarkSize = 0.5f;
    const Vector3& center = lastSeenPlayerPos;
    // 十字
    LineManager::GetInstance()->DrawLine(center + Vector3{-kLastSeenMarkSize, 0.0f, 0.0f}, center + Vector3{kLastSeenMarkSize, 0.0f, 0.0f}, kLastSeenColor);
    LineManager::GetInstance()->DrawLine(center + Vector3{0.0f, -kLastSeenMarkSize, 0.0f}, center + Vector3{0.0f, kLastSeenMarkSize, 0.0f}, kLastSeenColor);
    // 外周円
    constexpr int circleDiv = 16;
    for (int i = 0; i < circleDiv; ++i) {
        float a0 = (2.0f * kPI) * (float(i) / circleDiv);
        float a1 = (2.0f * kPI) * (float(i + 1) / circleDiv);
        Vector3 p0 = center + Vector3{std::cos(a0) * kLastSeenMarkSize, std::sin(a0) * kLastSeenMarkSize, 0.0f};
        Vector3 p1 = center + Vector3{std::cos(a1) * kLastSeenMarkSize, std::sin(a1) * kLastSeenMarkSize, 0.0f};
        LineManager::GetInstance()->DrawLine(p0, p1, kLastSeenColor);
    }
}

// 末尾に欠けていた関数を追加
Vector3 Enemy::GetCenterPosition() const {
    // 当たり判定中心（今はスケール考慮せずそのまま）
    return position;
}

void Enemy::DrawStateIcon() {
    // 状態に応じた簡易アイコンラインを頭上に描画
    if (!isAllive)
        return;
    Vector3 base = position;
    base.z += 0.0f; // 2D平面なのでそのまま
    Vector3 upPos = base + Vector3{0, 0, 0};
    // 色を状態で切替
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
    Vector3 p0 = base + Vector3{0, -s, 0};
    Vector3 p1 = base + Vector3{s, 0, 0};
    Vector3 p2 = base + Vector3{0, s, 0};
    Vector3 p3 = base + Vector3{-s, 0, 0};
    LineManager::GetInstance()->DrawLine(p0, p1, col);
    LineManager::GetInstance()->DrawLine(p1, p2, col);
    LineManager::GetInstance()->DrawLine(p2, p3, col);
    LineManager::GetInstance()->DrawLine(p3, p0, col);
}