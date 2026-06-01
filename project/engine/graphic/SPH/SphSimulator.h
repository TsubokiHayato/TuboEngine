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
#include "SphComputePipeline.h"
#include "InstancedMeshRenderer.h"
#include "LineManager.h"
#include <string>
#include <vector>
#undef min
#undef max

/// @brief SPH 流体シミュレーター (GPU Compute 版)
///
/// - 物理計算: 4 本の Compute Shader が全て GPU 上で実行
/// - CPU は初期化・パラメーター更新のみ
/// - 描画: UV 球メッシュ × GPU インスタンシング → 1 DrawCall
///         ParticlePSO パス (ParticleDraw) 内で Draw() を呼ぶこと
class SphSimulator {
public:
    struct Params {
        int   particleCount   = 1000;
        float smoothingRadius = 1.0f;
        float restDensity     = 14.0f;
        float stiffness       = 50.0f;
        float viscosity       = 8.0f;
        float particleMass    = 1.0f;
        float gravity         = -3.0f;
        float restitution     = 0.2f;
        float particleRadius  = 0.18f;
        float speedMax        = 5.0f;   // 色補間の最大速度
        TuboEngine::Math::Vector3 boundMin = {-5.0f, 0.0f, -5.0f};
        TuboEngine::Math::Vector3 boundMax = { 5.0f, 8.0f,  5.0f};
        TuboEngine::Math::Vector4 colorLow  = {0.4f, 0.7f, 1.0f, 1.0f};
        TuboEngine::Math::Vector4 colorHigh = {0.0f, 0.2f, 1.0f, 1.0f};
        int   substeps        = 3;
    };

    /// @param modelPath  球モデルのパス (デフォルト: Sphere.obj)
    /// @param texture    パーティクル PSO 用テクスチャ (Object3d 使用時は無視)
    void Initialize(const Params& params = {},
                    TuboEngine::Camera* camera = nullptr,
                    const std::string& modelPath = "Resources/Model/Sphere/Sphere.obj",
                    const std::string& texture   = "particle.png");
    void Update(float dt, TuboEngine::Camera* camera);
    void Draw();       // Object3DDraw() パス内で呼ぶ (InstancedMeshRenderer 使用)
    void DrawBounds(const TuboEngine::Math::Vector4& color = {0.3f, 0.8f, 1.0f, 1.0f});
    void DrawImGui();
    void Reset();
    void Finalize();

    Params& GetParams() { return params_; }

private:
    // 初期グリッド配置を CPU で生成して GPU にアップロード
    std::vector<SphParticle> GenerateInitialParticles() const;

    // 描画用 UV 球メッシュ
    void BuildGeometry();
    void EnsureRenderBuffers();

    // ---- データ ----
    Params                  params_;
    SphComputePipeline      compute_;      // GPU Compute 管理

    TuboEngine::Math::Matrix4x4 viewProj_ = {};

    InstancedMeshRenderer renderer_;   // Object3d パイプラインで 1 DrawCall 描画
};
