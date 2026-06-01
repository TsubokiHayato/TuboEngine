// SphCommon.hlsli — SPH 全シェーダー共通定義

static const float SPH_PI = 3.14159265359f;

// ---- GPU 上の粒子データ (SphParticle.h と完全一致) ----
struct SphParticle {
    float3 position;
    float  density;
    float3 velocity;
    float  pressure;
    float3 force;
    float  _pad;
};

// ---- GPU 上のインスタンスデータ (SphGPUInstance と一致) ----
struct SphInstance {
    float4x4 WVP;
    float4x4 World;
    float4   color;
};

// ---- シミュレーションパラメーター ----
cbuffer SphParams : register(b0) {
    int    g_ParticleCount;    // 粒子数
    float  g_H;                // smoothingRadius
    float  g_RestDensity;      // restDensity
    float  g_Stiffness;        // stiffness (Tait方程式係数)
    // 16 bytes
    float  g_Viscosity;        // viscosity
    float  g_Mass;             // particleMass
    float  g_Gravity;          // gravity (Y)
    float  g_Restitution;      // restitution
    // 32 bytes
    float3 g_BoundMin;         // boundMin
    float  g_Dt;               // dt per substep
    // 48 bytes
    float3 g_BoundMax;         // boundMax
    float  g_SpeedMax;         // 色補間最大速度
    // 64 bytes
    float4 g_ColorLow;         // colorLow
    // 80 bytes
    float4 g_ColorHigh;        // colorHigh
    // 96 bytes
    float  g_SurfaceTension;   // 表面張力係数 σ
    float  g_ParticleRadius;   // 描画半径
    float  _cbPad0;
    float  _cbPad1;
    // 112 bytes
    float4x4 g_ViewProj;       // view-projection matrix (16-byte aligned)
};

// ---- SPH カーネル ----
float KernelPoly6(float r2, float h) {
    if (r2 >= h * h) return 0.0f;
    float d = h * h - r2;
    return (315.0f / (64.0f * SPH_PI * pow(h, 9.0f))) * d * d * d;
}

float3 KernelSpikyGrad(float3 rij, float r, float h) {
    if (r >= h || r < 1e-6f) return float3(0, 0, 0);
    float d = h - r;
    return rij * (-(45.0f / (SPH_PI * pow(h, 6.0f))) * d * d / r);
}

float KernelViscLap(float r, float h) {
    if (r >= h) return 0.0f;
    return (45.0f / (SPH_PI * pow(h, 6.0f))) * (h - r);
}
