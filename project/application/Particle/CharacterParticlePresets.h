#pragma once
#include "engine/graphic/Particle/IParticleEmitter.h"
#include "Vector3.h"

// キャラクター共通パーティクルプリセット
// 各クラスの Initialize() でのボイラープレートを排除する
namespace CharacterParticlePresets {

inline ParticlePreset EnemyHit(const TuboEngine::Math::Vector3& pos = {}) {
    ParticlePreset p{};
    p.name        = "EnemyHit";
    p.texture     = "particle.png";
    p.maxInstances = 64;
    p.autoEmit    = false;
    p.burstCount  = 12;
    p.lifeMin     = 0.25f; p.lifeMax = 0.35f;
    p.scaleStart  = {0.15f, 0.15f, 0.15f};
    p.scaleEnd    = {0.05f, 0.05f, 0.05f};
    p.colorStart  = {1.0f, 0.6f, 0.2f, 0.9f};
    p.colorEnd    = {1.0f, 1.0f, 0.4f, 0.0f};
    p.center      = pos;
    return p;
}

inline ParticlePreset EnemyHitRing(const TuboEngine::Math::Vector3& pos = {}) {
    ParticlePreset p{};
    p.name        = "EnemyHitRing";
    p.texture     = "gradationLine.png";
    p.maxInstances = 16;
    p.autoEmit    = false;
    p.burstCount  = 1;
    p.lifeMin     = 0.25f; p.lifeMax = 0.45f;
    p.scaleStart  = {0.4f, 0.4f, 1.0f};
    p.scaleEnd    = {0.4f, 0.4f, 1.0f};
    p.colorStart  = {1.0f, 0.5f, 0.2f, 0.9f};
    p.colorEnd    = {1.0f, 0.9f, 0.6f, 0.0f};
    p.center      = pos;
    return p;
}

inline ParticlePreset EnemyDeath(const TuboEngine::Math::Vector3& pos = {}) {
    ParticlePreset p{};
    p.name        = "EnemyDeath";
    p.texture     = "gradationLine.png";
    p.maxInstances = 8;
    p.autoEmit    = false;
    p.burstCount  = 1;
    p.lifeMin     = 0.6f; p.lifeMax = 0.9f;
    p.scaleStart  = {0.6f, 0.6f, 0.6f};
    p.scaleEnd    = {1.4f, 1.4f, 1.4f};
    p.colorStart  = {1.0f, 0.3f, 0.2f, 0.9f};
    p.colorEnd    = {1.0f, 0.8f, 0.1f, 0.0f};
    p.center      = pos;
    return p;
}

inline ParticlePreset EnemyMist(const TuboEngine::Math::Vector3& pos = {}) {
    ParticlePreset p{};
    p.name        = "EnemyMist";
    p.texture     = "particle.png";
    p.maxInstances = 256;
    p.autoEmit    = false;
    p.burstCount  = 12;
    p.lifeMin     = 0.4f; p.lifeMax = 0.7f;
    p.scaleStart  = {0.15f, 0.15f, 0.15f};
    p.scaleEnd    = {0.5f,  0.5f,  0.5f};
    p.colorStart  = {0.7f, 0.7f, 0.7f, 0.6f};
    p.colorEnd    = {0.3f, 0.3f, 0.3f, 0.0f};
    p.velMin      = {-1.2f, -1.2f, -1.2f};
    p.velMax      = { 1.2f,  1.2f,  1.2f};
    p.center      = pos;
    return p;
}

inline ParticlePreset RushCharge(const TuboEngine::Math::Vector3& pos = {}) {
    ParticlePreset p{};
    p.name        = "RushCharge";
    p.texture     = "particle.png";
    p.maxInstances = 128;
    p.autoEmit    = false;
    p.burstCount  = 4;
    p.lifeMin     = 0.1f; p.lifeMax = 0.25f;
    p.scaleStart  = {0.2f, 0.2f, 0.2f};
    p.scaleEnd    = {0.05f, 0.05f, 0.05f};
    p.colorStart  = {1.0f, 0.7f, 0.2f, 1.0f};
    p.colorEnd    = {1.0f, 0.4f, 0.1f, 0.0f};
    p.velMin      = {-3.0f, -3.0f, -3.0f};
    p.velMax      = { 3.0f,  3.0f,  3.0f};
    p.gravity     = {0, 0, 0};
    p.center      = pos;
    return p;
}

inline ParticlePreset RushTrail(const TuboEngine::Math::Vector3& pos = {}) {
    ParticlePreset p{};
    p.name        = "RushTrail";
    p.texture     = "particle.png";
    p.maxInstances = 256;
    p.autoEmit    = false;
    p.burstCount  = 2;
    p.lifeMin     = 0.3f; p.lifeMax = 0.5f;
    p.scaleStart  = {0.4f, 0.4f, 0.4f};
    p.scaleEnd    = {0.8f, 0.8f, 0.8f};
    p.colorStart  = {0.8f, 0.8f, 0.8f, 0.5f};
    p.colorEnd    = {0.6f, 0.6f, 0.6f, 0.0f};
    p.velMin      = {-0.6f, -0.6f, -0.6f};
    p.velMax      = { 0.6f,  0.6f,  0.6f};
    p.gravity     = {0, 0, 0};
    p.center      = pos;
    return p;
}

inline ParticlePreset PlayerTrail(const TuboEngine::Math::Vector3& pos = {}) {
    ParticlePreset p{};
    p.name                 = "PlayerTrail";
    p.texture              = "circle2.png";
    p.autoEmit             = true;
    p.emitRate             = 60.0f;
    p.lifeMin              = 0.35f; p.lifeMax = 0.6f;
    p.scaleStart           = {0.7f, 0.7f, 0.7f};
    p.scaleEnd             = {0.6f, 0.6f, 0.6f};
    p.colorStart           = {0.6f, 0.8f, 1.0f, 0.9f};
    p.colorEnd             = {0.2f, 0.4f, 1.0f, 0.0f};
    p.maxInstances         = 512;
    p.billboard            = true;
    p.simulateInWorldSpace = true;
    p.center               = pos;
    return p;
}

inline ParticlePreset PlayerDashRing(const TuboEngine::Math::Vector3& pos = {}) {
    ParticlePreset p{};
    p.name                 = "PlayerDashRing";
    p.texture              = "gradationLine.png";
    p.maxInstances         = 16;
    p.autoEmit             = false;
    p.burstCount           = 1;
    p.lifeMin              = 0.35f; p.lifeMax = 0.6f;
    p.scaleStart           = {0.6f, 0.6f, 1.0f};
    p.scaleEnd             = {1.2f, 1.2f, 1.0f};
    p.colorStart           = {0.9f, 0.95f, 1.0f, 0.85f};
    p.colorEnd             = {0.9f, 0.95f, 1.0f, 0.0f};
    p.simulateInWorldSpace = false;
    p.center               = pos;
    return p;
}

} // namespace CharacterParticlePresets
