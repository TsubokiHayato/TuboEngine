#pragma once
#include "DirectXCommon.h"
#include "SrvManager.h"
#include "SphParticle.h"
#include "Matrix.h"
#include "Matrix4x4.h"
#include "Vector4.h"
#include <vector>
#include <wrl/client.h>
#undef min
#undef max

/// @brief GPU 上の SPH インスタンスデータ (Particle PSO と同レイアウト)
struct SphGPUInstance {
    TuboEngine::Math::Matrix4x4 WVP;
    TuboEngine::Math::Matrix4x4 World;
    TuboEngine::Math::Vector4   color;
};

/// @brief SPH シミュレーションパラメーター (GPU 定数バッファ / 112 bytes)
struct alignas(16) SphGpuParams {
    int    particleCount;
    float  h;
    float  restDensity;
    float  stiffness;
    // 16 bytes
    float  viscosity;
    float  mass;
    float  gravity;
    float  restitution;
    // 32 bytes
    float  boundMinX, boundMinY, boundMinZ;
    float  dt;
    // 48 bytes
    float  boundMaxX, boundMaxY, boundMaxZ;
    float  speedMax;
    // 64 bytes
    float  colorLow[4];   // 80
    float  colorHigh[4];  // 96
    TuboEngine::Math::Matrix4x4 viewProj; // 160 bytes total
};

/// @brief SPH GPU コンピュートパイプライン
///
/// 4 本の Compute Shader を管理:
///   1. CsDensity    — 密度・圧力
///   2. CsForce      — 圧力・粘性・重力
///   3. CsIntegrate  — 積分 + 境界
///   4. CsPrepareInstances — WVP 行列を GPU 上で計算
class SphComputePipeline {
public:
    void Initialize(int particleCount);
    void Finalize();

    /// 初期粒子データを GPU バッファにアップロード
    void UploadInitialParticles(const std::vector<SphParticle>& particles);

    /// 1フレーム分の SPH 計算 (Dispatch × substeps × 3pass + PrepareInstances)
    void Dispatch(const SphGpuParams& params, int substeps = 3);

    /// DrawInstanced 用インスタンシング SRV インデックス (ParticlePSO 向け)
    int GetInstancingSrvIndex() const { return instancingSrvIndex_; }
    int GetParticleCount()      const { return particleCount_; }

    /// Object3d インスタンシング用 — InstanceData バッファの GPU アドレス
    /// SphComputePipeline::Dispatch() 後に有効 (UAV→SRV 遷移済み)
    D3D12_GPU_VIRTUAL_ADDRESS GetInstanceBufferGPUAddr() const {
        return instanceBuf_ ? instanceBuf_->GetGPUVirtualAddress() : 0;
    }

private:
    void CreateRootSignature();
    void CreateComputePSO(const wchar_t* shaderPath,
                           Microsoft::WRL::ComPtr<ID3D12PipelineState>& outPso);
    void CreateDefaultBuffer(UINT64 size,
                              Microsoft::WRL::ComPtr<ID3D12Resource>& outBuf);
    void CreateParamsBuffer();
    void UAVBarrier(ID3D12GraphicsCommandList* cmd, ID3D12Resource* resource);
    void TransitionBarrier(ID3D12GraphicsCommandList* cmd, ID3D12Resource* resource,
                            D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after);

    // ---- PSO ----
    Microsoft::WRL::ComPtr<ID3D12RootSignature>  rootSig_;
    Microsoft::WRL::ComPtr<ID3D12PipelineState>  psoDensity_;
    Microsoft::WRL::ComPtr<ID3D12PipelineState>  psoForce_;
    Microsoft::WRL::ComPtr<ID3D12PipelineState>  psoIntegrate_;
    Microsoft::WRL::ComPtr<ID3D12PipelineState>  psoPrepare_;

    // ---- GPU バッファ ----
    Microsoft::WRL::ComPtr<ID3D12Resource> particleBuf_;   // RWStructuredBuffer
    Microsoft::WRL::ComPtr<ID3D12Resource> instanceBuf_;   // RWStructuredBuffer
    Microsoft::WRL::ComPtr<ID3D12Resource> paramsCbuf_;    // ConstantBuffer (UPLOAD)
    SphGpuParams* paramsMapped_ = nullptr;

    // ---- ディスクリプタ ----
    int particleUavIndex_   = -1;  // UAV  : u0 (particle buffer)
    int instanceUavIndex_   = -1;  // UAV  : u1 (instance buffer)
    int instancingSrvIndex_ = -1;  // SRV  : VS が読む

    int particleCount_ = 0;
    bool initialized_        = false;
    bool instanceBufInSRV_  = false;  // true = NON_PIXEL_SHADER_RESOURCE, false = UAV/COMMON
};
