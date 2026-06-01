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

/// @brief SDF 障害物の記述 (CPU 側インターフェース)
struct SdfObstacle {
    enum class Type { Sphere, Box } type;
    TuboEngine::Math::Vector3 center;
    TuboEngine::Math::Vector3 halfExtents;  // sphere: x=radius; box: 各軸の半辺長
    std::string label;

    // ---- Phase 2: 剛体物理 ----
    bool    dynamic  = false;  // true → 浮力・抵抗・重力で動く
    float   mass     = 1.0f;  // 質量 (kg 相当)
    TuboEngine::Math::Vector3 velocity = {};  // 現在の線速度
};

/// @brief SPH 流体シミュレーター (GPU Compute 版)
///
/// - 物理計算: 4 本の Compute Shader が全て GPU 上で実行
/// - CPU は初期化・パラメーター更新のみ
/// - 描画: UV 球メッシュ × GPU インスタンシング → 1 DrawCall
///         ParticlePSO パス (ParticleDraw) 内で Draw() を呼ぶこと
class SphSimulator {
public:
    struct Params {
        int   particleCount   = 10000;          // 粒子数
        float smoothingRadius = 1.0f;           // 影響半径 h
        float restDensity     = 6.0f;           // 静止密度 (カーネル積分から計算した実際の値)
        float stiffness       = 200.0f;         // 圧力剛性 (Tait方程式用)
        float viscosity       = 10.0f;           // 粘性係数 (水は低粘性)
        float particleMass    = 1.0f;           // 粒子質量
        float gravity         = -9.8f;          // 重力
        float restitution     = 0.02f;          // 壁反発係数 (液体は跳ねない)
        float particleRadius  = 0.3f;          // 描画半径
        float speedMax        = 5.0f;           // 色補間の最大速度
        float xsphCoeff       = 0.15f;          // XSPH 速度補正係数 (0=無効, 0.1〜0.3が標準)
        float surfaceTension  = 2.0f;           // 表面張力係数 (0=無効, 大きいほど水滴が丸まる)
        // ---- 外力 (力点から放射状に押す/引く) ----
        bool  extForceActive   = false;
        TuboEngine::Math::Vector3 extForcePos = {0.0f, 5.0f, 0.0f};
        float extForceRadius   = 4.0f;
        float extForceStrength = 50.0f;         // 正=押し出し, 負=引き寄せ
        // ボックスは流体体積(N*m/ρ₀≈1667unit³)の3倍以上必要
        // 16×20×16=5120unit³ → 流体が底部1/3を満たす水槽になる
        TuboEngine::Math::Vector3 boundMin = {-8.0f,  0.0f, -8.0f};
        TuboEngine::Math::Vector3 boundMax = { 8.0f, 20.0f,  8.0f};
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

    // ---- SDF 障害物管理 ----
    void AddSphere(const TuboEngine::Math::Vector3& center, float radius,
                   const std::string& label = "");
    void AddBox(const TuboEngine::Math::Vector3& center,
                const TuboEngine::Math::Vector3& halfExtents,
                const std::string& label = "");
    // 物理剛体として追加するショートカット
    void AddDynamicSphere(const TuboEngine::Math::Vector3& center, float radius,
                          float mass, const std::string& label = "");
    void AddDynamicBox(const TuboEngine::Math::Vector3& center,
                       const TuboEngine::Math::Vector3& halfExtents,
                       float mass, const std::string& label = "");
    void ClearObstacles();

    Params& GetParams() { return params_; }

    // プリセット (水/ハチミツ/スライム をワンクリック適用)
    enum class Preset { Water, Honey, Slime };
    void ApplyPreset(Preset preset);

private:
    // 初期グリッド配置を CPU で生成して GPU にアップロード
    std::vector<SphParticle> GenerateInitialParticles() const;
    void UpdateMouseForce();      // マウスドラッグで外力点を操作
    void IntegrateRigidBodies(float dt);  // 剛体の物理積分 (浮力・抵抗・重力)

    // 描画用 UV 球メッシュ
    void BuildGeometry();
    void EnsureRenderBuffers();

    // ---- データ ----
    Params                  params_;
    SphComputePipeline      compute_;      // GPU Compute 管理

    TuboEngine::Math::Matrix4x4 viewProj_ = {};

    InstancedMeshRenderer renderer_;   // Object3d パイプラインで 1 DrawCall 描画

    // ---- SDF 障害物 ----
    std::vector<SdfObstacle> obstacles_;

    // ---- 再生コントロール ----
    bool  paused_            = false;  // 一時停止中は物理を進めない
    bool  stepOnce_          = false;  // コマ送り: 次の1フレームだけ進める
    float timeScale_         = 1.0f;   // 再生速度 (0=停止, 1=等速)
    // ---- マウス外力 ----
    bool  mouseForceEnabled_ = false;  // マウスドラッグで外力を操作するか
    bool  mouseDriving_      = false;  // 現在マウスが外力を駆動中か
};
