// SphCommon.hlsli — SPH 全シェーダー共通定義

static const float SPH_PI = 3.14159265359f;

// ---- GPU 上の粒子データ (SphParticle.h と完全一致, 64 bytes) ----
struct SphParticle {
    float3 position;
    float  density;
    float3 velocity;
    float  pressure;
    float3 force;
    float  _pad0;
    float3 xsph;       // XSPH 速度補正
    float  _pad1;
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
    float  g_Stiffness;        // stiffness
    float  g_Viscosity;        // viscosity
    float  g_Mass;             // particleMass
    float  g_Gravity;          // gravity (Y)
    float  g_Restitution;      // restitution
    float3 g_BoundMin;         // boundMin
    float  g_Dt;               // dt per substep
    float3 g_BoundMax;         // boundMax
    float  g_SpeedMax;         // 色補間最大速度
    float4 g_ColorLow;         // colorLow
    float4 g_ColorHigh;        // colorHigh
    float  g_ParticleRadius;   // 粒子描画半径
    float  g_XsphCoeff;        // XSPH 速度補正係数 ε
    float  _cbPad1, _cbPad2;
    float4x4 g_ViewProj;       // view-projection matrix (16-byte aligned)
    // ---- 空間ハッシュ (近傍探索高速化) ----
    int3   g_GridDim;          // グリッド各軸セル数
    float  g_CellSize;         // セルサイズ (= 初期 smoothingRadius)
    float3 g_GridMin;          // グリッド原点 (= 初期 boundMin)
    int    g_MaxPerCell;       // セルあたり最大粒子数
    // ---- 外力 (力点から放射状) ----
    float3 g_ExtForcePos;      // 力点中心
    float  g_ExtForceRadius;   // 影響半径
    float  g_ExtForceStrength; // 強さ (正=押し出し, 負=引き寄せ)
    int    g_ExtForceActive;   // 有効フラグ
    float  g_SurfaceTension;   // 表面張力係数 σ
    float  _extPad1;
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

// ---- 空間ハッシュ ヘルパー ----
// 粒子位置からグリッドセル座標を求める (範囲外は端セルにクランプ)
int3 SphCellCoord(float3 pos) {
    int3 c = (int3)floor((pos - g_GridMin) / g_CellSize);
    return clamp(c, int3(0, 0, 0), g_GridDim - int3(1, 1, 1));
}

// セル座標を 1 次元インデックスに変換
int SphCellIndex(int3 c) {
    return c.x + c.y * g_GridDim.x + c.z * g_GridDim.x * g_GridDim.y;
}
