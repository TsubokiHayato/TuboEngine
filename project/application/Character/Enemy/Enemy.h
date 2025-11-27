#pragma once
#include "Character/BaseCharacter.h"
#include "Bullet/Enemy/EnemyNormalBullet.h"
#include "MapChip/MapChipField.h"
#include "Particle.h"
#include "ParticleEmitter.h"
// 演出用: 前方宣言のみで十分
class IParticleEmitter;

class Player;
class Sprite; // スプライト前方宣言

class Enemy : public BaseCharacter {
public:
    Enemy();
    ~Enemy() override;
    void Initialize() override;
    void Update() override;
    void Draw() override;
    void ParticleDraw();
    void DrawImGui();
    void Move();
    bool CanSeePlayer();
    void DrawViewCone();
    void DrawLastSeenMark();
    void DrawStateIcon();
    void EmitHitParticle();
    void EmitDeathParticle();
    void OnCollision(Collider* other) override;
    Vector3 GetCenterPosition() const override;

    void SetCamera(Camera* camera) { camera_ = camera; }
    Vector3 GetPosition() const { return position; }
    void SetPosition(const Vector3& pos) { position = pos; }
    Vector3 GetRotation() const { return rotation; }
    void SetRotation(const Vector3& rot) { rotation = rot; }
    Vector3 GetScale() const { return scale; }
    void SetScale(const Vector3& scl) { scale = scl; }
    bool GetIsAllive() const { return isAllive; }
    void SetIsAlive(bool alive) { isAllive = alive; }
    void SetPlayer(Player* player) { player_ = player; }
    void SetMapChipField(MapChipField* field) { mapChipField = field; }
    int GetHP() const { return HP; }
    int GetMaxHP() const { return 10; }

    enum class State { Idle, Alert, LookAround, Patrol, Chase, Attack };

private:
    Vector3 position;
    Vector3 rotation;
    Vector3 scale = {1.0f, 1.0f, 1.0f};
    Vector3 velocity;
    float turnSpeed_ = 0.1f;
    float moveSpeed_ = 0.08f;
    float shootDistance_ = 7.0f;
    float moveStartDistance_ = 15.0f;
    int HP = 100;
    bool isAllive = true;
    bool isHit = false;
    bool wasHit = false;
    std::unique_ptr<Object3d> object3d;
    State state_ = State::Idle;
    float kViewAngleDeg = 90.0f;
    float kViewDistance = 15.0f;
    int kViewLineDiv = 16;
    Vector4 kViewColor = {1.0f, 1.0f, 0.0f, 0.7f};
    Vector3 lastSeenPlayerPos = {0.0f, 0.0f, 0.0f};
    float lastSeenTimer = 0.0f;
    float kLastSeenDuration = 3.0f;
    std::unique_ptr<EnemyNormalBullet> bullet;
    float lookAroundBaseAngle = 0.0f;
    float lookAroundTargetAngle = 0.0f;
    int lookAroundDirection = 1;
    float lookAroundAngleWidth = 1.25f;
    float lookAroundSpeed = 0.06f;
    int lookAroundCount = 0;
    int lookAroundMaxCount = 4;
    bool lookAroundInitialized = false;
    float idleLookAroundIntervalSec = 4.0f;
    float idleLookAroundTimer = 0.0f;
    enum class IdleBackPhase { None, ToBack, Hold, Return };
    IdleBackPhase idleBackPhase_ = IdleBackPhase::None;
    float idleBackTurnSpeed = 0.03f;
    float idleBackHoldSec = 0.5f;
    float idleBackHoldTimer = 0.0f;
    float idleBackStartAngle = 0.0f;
    float idleBackTargetAngle = 0.0f;
    bool showSurpriseIcon_ = false;
    Vector4 stateIconColor = {1, 0, 0, 1};
    float stateIconSize = 0.8f;
    float stateIconHeight = 3.0f;
    float stateIconLineWidth = 0.08f;
    float bulletTimer_ = 0.0f;
    bool wantShoot_ = false;
    std::vector<Vector3> currentPath_;
    size_t pathCursor_ = 0;
    int lastPathGoalIndex_ = -1;
    float waypointArriveEps_ = 0.15f;
    IParticleEmitter* hitEmitter_ = nullptr;
    IParticleEmitter* deathEmitter_ = nullptr;
    bool deathEffectPlayed_ = false;
    Camera* camera_ = nullptr;
    MapChipField* mapChipField = nullptr;
    Player* player_ = nullptr;
    void ClearPath() { currentPath_.clear(); pathCursor_ = 0; lastPathGoalIndex_ = -1; }
    bool BuildPathTo(const Vector3& worldGoal);
};
