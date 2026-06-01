// CsForce.hlsl — 圧力・粘性・重力計算パス
#include "SphCommon.hlsli"

RWStructuredBuffer<SphParticle> g_Particles : register(u0);

[numthreads(256, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    uint i = tid.x;
    if ((int)i >= g_ParticleCount) return;

    float3 pi  = g_Particles[i].position;
    float  rho = g_Particles[i].density;
    float  pri = g_Particles[i].pressure;
    float3 vi  = g_Particles[i].velocity;

    float3 fPressure  = float3(0, 0, 0);
    float3 fViscosity = float3(0, 0, 0);

    for (int j = 0; j < g_ParticleCount; ++j) {
        if (j == (int)i) continue;

        float3 rij = pi - g_Particles[j].position;
        float  r   = length(rij);
        if (r >= g_H || r < 1e-6f) continue;

        float  rhoj = g_Particles[j].density;
        float  prj  = g_Particles[j].pressure;
        float3 vj   = g_Particles[j].velocity;

        // 圧力項
        fPressure  += KernelSpikyGrad(rij, r, g_H) *
                      (-g_Mass * (pri + prj) / (2.0f * rhoj));
        // 粘性項
        fViscosity += (vj - vi) * (g_Viscosity * g_Mass / rhoj * KernelViscLap(r, g_H));
    }

    float3 fGravity = float3(0.0f, g_Gravity * rho, 0.0f);
    g_Particles[i].force = fPressure + fViscosity + fGravity;
}
