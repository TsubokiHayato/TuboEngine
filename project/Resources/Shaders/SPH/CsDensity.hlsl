// CsDensity.hlsl — 密度・圧力計算パス (O(N²) on GPU)
#include "SphCommon.hlsli"

RWStructuredBuffer<SphParticle> g_Particles : register(u0);

[numthreads(256, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    uint i = tid.x;
    if ((int)i >= g_ParticleCount) return;

    float3 pi = g_Particles[i].position;
    float density = 0.0f;

    for (int j = 0; j < g_ParticleCount; ++j) {
        float3 rij = pi - g_Particles[j].position;
        float  r2  = dot(rij, rij);
        density += g_Mass * KernelPoly6(r2, g_H);
    }

    // ---- ミラーパーティクル法 (Surface Deficiency 対策) ----
    // 各軸で最も近い壁を選び、面(3)・辺(3)・角(1)の鏡像粒子の密度寄与を加算する。
    // 鏡像距離² は各軸の (壁からの距離×2)² の和。KernelPoly6 が h 以遠を 0 にするため
    // 遠い壁の寄与は自動的に無視される。辺・角まで含めることで隅の密度不足も補正される。
    {
        float3 p  = g_Particles[i].position;
        float  ax = min(p.x - g_BoundMin.x, g_BoundMax.x - p.x);
        float  ay = min(p.y - g_BoundMin.y, g_BoundMax.y - p.y);
        float  az = min(p.z - g_BoundMin.z, g_BoundMax.z - p.z);
        float  bx = (2.0f * ax) * (2.0f * ax);
        float  by = (2.0f * ay) * (2.0f * ay);
        float  bz = (2.0f * az) * (2.0f * az);
        // 面 (3)
        density += g_Mass * KernelPoly6(bx,           g_H);
        density += g_Mass * KernelPoly6(by,           g_H);
        density += g_Mass * KernelPoly6(bz,           g_H);
        // 辺 (3)
        density += g_Mass * KernelPoly6(bx + by,      g_H);
        density += g_Mass * KernelPoly6(by + bz,      g_H);
        density += g_Mass * KernelPoly6(bx + bz,      g_H);
        // 角 (1)
        density += g_Mass * KernelPoly6(bx + by + bz, g_H);
    }

    density = max(density, 1e-6f);
    g_Particles[i].density = density;

    // Tait方程式: p = k * ((ρ/ρ₀)^7 - 1)
    // 線形式より非圧縮性が高く、液体らしい動きになる
    float ratio  = density / g_RestDensity;
    float ratio2 = ratio  * ratio;
    float ratio4 = ratio2 * ratio2;
    float ratio7 = ratio4 * ratio2 * ratio;
    g_Particles[i].pressure = g_Stiffness * (ratio7 - 1.0f);
}
