// CsIntegrate.hlsl — 速度・位置の積分 + AABB 境界反射
#include "SphCommon.hlsli"

RWStructuredBuffer<SphParticle> g_Particles : register(u0);

[numthreads(256, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    uint i = tid.x;
    if ((int)i >= g_ParticleCount) return;

    float3 pos = g_Particles[i].position;
    float3 vel = g_Particles[i].velocity;
    float3 f   = g_Particles[i].force;
    float  rho = g_Particles[i].density;

    // セミインプリシット Euler
    float3 accel = f / rho;
    vel += accel * g_Dt;
    pos += vel   * g_Dt;

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
