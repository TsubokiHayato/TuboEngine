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

    density = max(density, 1e-6f);
    g_Particles[i].density  = density;
    g_Particles[i].pressure = g_Stiffness * (density - g_RestDensity);
}
