// CsIntegrate.hlsl — 速度・位置の積分 + AABB 境界反射
#include "SphCommon.hlsli"

RWStructuredBuffer<SphParticle> g_Particles : register(u0);

[numthreads(256, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    uint i = tid.x;
    if ((int)i >= g_ParticleCount) return;

    float3 pos  = g_Particles[i].position;
    float3 vel  = g_Particles[i].velocity;
    float3 f    = g_Particles[i].force;
    float  rho  = g_Particles[i].density;
    float3 xsph = g_Particles[i].xsph;

    // セミインプリシット Euler + XSPH 位置補正
    // 速度本体は補正せず、位置更新にのみ近傍平均速度を加える (Monaghan XSPH)
    // → 粒子が個別にバラけず、連続した流体のように動く
    float3 accel = f / rho;
    vel += accel * g_Dt;
    pos += (vel + xsph) * g_Dt;

    // AABB 境界反射
    if (pos.x < g_BoundMin.x) { pos.x = g_BoundMin.x; vel.x =  abs(vel.x) * g_Restitution; }
    if (pos.x > g_BoundMax.x) { pos.x = g_BoundMax.x; vel.x = -abs(vel.x) * g_Restitution; }
    if (pos.y < g_BoundMin.y) { pos.y = g_BoundMin.y; vel.y =  abs(vel.y) * g_Restitution; }
    if (pos.y > g_BoundMax.y) { pos.y = g_BoundMax.y; vel.y = -abs(vel.y) * g_Restitution; }
    if (pos.z < g_BoundMin.z) { pos.z = g_BoundMin.z; vel.z =  abs(vel.z) * g_Restitution; }
    if (pos.z > g_BoundMax.z) { pos.z = g_BoundMax.z; vel.z = -abs(vel.z) * g_Restitution; }

    g_Particles[i].position = pos;
    g_Particles[i].velocity = vel;
}
