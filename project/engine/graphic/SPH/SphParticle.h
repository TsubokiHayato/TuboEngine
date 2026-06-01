#pragma once
#include "Vector3.h"

/// @brief SPH シミュレーション用パーティクル (CPU 側データ)
struct SphParticle {
    TuboEngine::Math::Vector3 position  = {0.0f, 0.0f, 0.0f};
    TuboEngine::Math::Vector3 velocity  = {0.0f, 0.0f, 0.0f};
    TuboEngine::Math::Vector3 force     = {0.0f, 0.0f, 0.0f};
    float density  = 0.0f;   // ρ
    float pressure = 0.0f;   // p
};
