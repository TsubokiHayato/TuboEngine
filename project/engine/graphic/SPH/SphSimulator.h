#pragma once
#include "DirectXCommon.h"
#include "SrvManager.h"
#include "TextureManager.h"
#include "Camera.h"
#include "Matrix.h"
#include "Vector3.h"
#include "Vector4.h"
#include "Material.h"
#include "VertexData.h"
#include "SphParticle.h"
#include <string>
#include <vector>
#undef min
#undef max

/// @brief ParticlePSO と同じ StructuredBuffer レイアウト
struct SphGPUInstance {
    TuboEngine::Math::Matrix4x4 WVP;
    TuboEngine::Math::Matrix4x4 World;
    TuboEngine::Math::Vector4   color;
};

/// @brief Smoothed Particle Hydrodynamics シミュレーター
///
/// パーティクルシステムとは独立した流体シミュレーション。
/// 既存の ParticlePSO をそのまま利用して描画する。
///
/// 使い方:
///   SphSimulator sph;
///   sph.Initialize();               // Initialize() で DirectX12 リソースを確保
///   // 毎フレーム
///   sph.Update(dt, camera);         // SPH 計算 + GPU バッファ更新
///   sph.Draw();                     // ParticlePSO が有効な描画パス内で呼ぶ
///   sph.DrawImGui();                // ImGui ウィンドウ
class SphSimulator {
public:
    /// @brief ImGui / コード側から動的に変更できるパラメーター
    struct Params {
        int   particleCount   = 256;            // 粒子数
        float smoothingRadius = 1.0f;           // 影響半径 h
        float restDensity     = 1000.0f;        // 静止密度 ρ₀
        float stiffness       = 1000.0f;        // 圧力剛性 k
        float viscosity       = 0.4f;           // 粘性係数 μ
        float particleMass    = 0.02f;          // 粒子質量 m
        float gravity         = -9.8f;          // 重力加速度 (Y 方向)
        float restitution     = 0.15f;          // 壁反発係数
        float particleRadius  = 0.15f;          // 描画半径
        TuboEngine::Math::Vector3 boundMin = {-3.0f, 0.0f, -3.0f};
        TuboEngine::Math::Vector3 boundMax = { 3.0f, 6.0f,  3.0f};
        TuboEngine::Math::Vector4 colorLow  = {0.3f, 0.6f, 1.0f, 0.85f}; // 低速
        TuboEngine::Math::Vector4 colorHigh = {0.0f, 0.2f, 0.9f, 1.0f};  // 高速
    };

    void Initialize(const Params& params = {}, const std::string& texture = "particle.png");
    void Update(float dt, const TuboEngine::Camera* camera);
    void Draw();        // ParticlePSO が有効な描画パス内で呼ぶこと
    void DrawImGui();
    void Reset();       // 粒子を初期配置に戻す
    void Finalize();

    Params& GetParams()                            { return params_; }
    const std::vector<SphParticle>& GetParticles() const { return particles_; }
    uint32_t GetInstanceCount()                    const { return instanceCount_; }

private:
    // ---- SPH パイプライン ----
    void ComputeDensityPressure();
    void ComputeForces();
    void Integrate(float dt);
    void HandleBoundaries();

    // ---- SPH カーネル関数 ----
    float               KernelPoly6   (float r2, float h) const;
    TuboEngine::Math::Vector3 KernelSpikyGrad(
        const TuboEngine::Math::Vector3& rij, float r, float h) const;
    float               KernelViscLap (float r,  float h) const;

    // ---- DirectX12 ----
    void BuildGeometry();   // ビルボード用クワッド頂点生成
    void EnsureBuffers();
    void UploadInstances(
        const TuboEngine::Math::Matrix4x4& viewProj,
        const TuboEngine::Math::Matrix4x4& billboard);

    // ---- メンバー ----
    Params params_;
    std::vector<SphParticle> particles_;

    Microsoft::WRL::ComPtr<ID3D12Resource> vb_;
    Microsoft::WRL::ComPtr<ID3D12Resource> instancing_;
    Microsoft::WRL::ComPtr<ID3D12Resource> material_;
    TuboEngine::Material* materialPtr_   = nullptr;
    SphGPUInstance*       instancingPtr_ = nullptr;

    int      textureSrvIndex_    = -1;
    int      instancingSrvIndex_ = -1;
    uint32_t instanceCount_      = 0;
    uint32_t allocatedCount_     = 0;
    bool     buffersReady_       = false;

    std::vector<TuboEngine::VertexData> vertices_;
    std::string texturePath_;

    TuboEngine::Math::Matrix4x4 viewProj_  = {};
    TuboEngine::Math::Matrix4x4 billboard_ = {};
};
