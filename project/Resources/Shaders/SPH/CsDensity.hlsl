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
    // 各壁の鏡像位置に仮想粒子を置き、境界近くの密度不足を補正する
    // 鏡像粒子との距離 = 2 * (粒子と壁の距離)
    {
        float3 p = g_Particles[i].position;
        float dx, dy, dz;
        // X 壁
        dx = p.x - g_BoundMin.x; if (dx < g_H) density += g_Mass * KernelPoly6(4.0f*dx*dx, g_H);
        dx = g_BoundMax.x - p.x; if (dx < g_H) density += g_Mass * KernelPoly6(4.0f*dx*dx, g_H);
        // Y 壁 (床・天井)
        dy = p.y - g_BoundMin.y; if (dy < g_H) density += g_Mass * KernelPoly6(4.0f*dy*dy, g_H);
        dy = g_BoundMax.y - p.y; if (dy < g_H) density += g_Mass * KernelPoly6(4.0f*dy*dy, g_H);
        // Z 壁
        dz = p.z - g_BoundMin.z; if (dz < g_H) density += g_Mass * KernelPoly6(4.0f*dz*dz, g_H);
        dz = g_BoundMax.z - p.z; if (dz < g_H) density += g_Mass * KernelPoly6(4.0f*dz*dz, g_H);
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
