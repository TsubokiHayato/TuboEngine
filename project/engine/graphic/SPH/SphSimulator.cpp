#include "SphSimulator.h"
#include "ImGuiManager.h"
#include <algorithm>
#include <cmath>
#include <cstring>
#include <numbers>
#undef min
#undef max

static constexpr float kPi = std::numbers::pi_v<float>;

// ============================================================
//  Initialize
// ============================================================
void SphSimulator::Initialize(const Params& params, TuboEngine::Camera* camera,
                                const std::string& modelPath, const std::string& texture)
{
    params_ = params;

    viewProj_ = TuboEngine::Math::MakeIdentity4x4();

    // GPU Compute パイプライン
    compute_.Initialize(params_.particleCount);

    // 初期粒子データを GPU にアップロード
    auto initialParticles = GenerateInitialParticles();
    compute_.UploadInitialParticles(initialParticles);

    // InstancedMeshRenderer — Object3d パイプライン + 1 DrawCall
    renderer_.Initialize(modelPath, params_.particleCount, camera);
    renderer_.SetLightType(1);   // Phong
}

// ============================================================
//  Finalize
// ============================================================
void SphSimulator::Finalize() {
    renderer_.Finalize();
    compute_.Finalize();
}

// ============================================================
//  Reset — 粒子を初期配置に戻して GPU に再アップロード
// ============================================================
void SphSimulator::Reset() {
    auto initialParticles = GenerateInitialParticles();
    compute_.UploadInitialParticles(initialParticles);
}

// ============================================================
//  Update — GPU Dispatch (CPU は ViewProj だけ渡す)
// ============================================================
void SphSimulator::Update(float dt, TuboEngine::Camera* camera) {
    if (camera) {
        viewProj_ = camera->GetViewProjectionMatrix();
    }
    renderer_.Update(camera);

    // GPU パラメーターを組み立て
    SphGpuParams gp{};
    gp.particleCount = params_.particleCount;
    gp.h             = params_.smoothingRadius;
    gp.restDensity   = params_.restDensity;
    gp.stiffness     = params_.stiffness;
    gp.viscosity     = params_.viscosity;
    gp.mass          = params_.particleMass;
    gp.gravity       = params_.gravity;
    gp.restitution   = params_.restitution;
    gp.dt            = dt / float(params_.substeps);
    gp.boundMinX = params_.boundMin.x;
    gp.boundMinY = params_.boundMin.y;
    gp.boundMinZ = params_.boundMin.z;
    gp.boundMaxX = params_.boundMax.x;
    gp.boundMaxY = params_.boundMax.y;
    gp.boundMaxZ = params_.boundMax.z;
    gp.speedMax       = params_.speedMax;
    gp.colorLow[0]  = params_.colorLow.x;  gp.colorLow[1]  = params_.colorLow.y;
    gp.colorLow[2]  = params_.colorLow.z;  gp.colorLow[3]  = params_.colorLow.w;
    gp.colorHigh[0] = params_.colorHigh.x; gp.colorHigh[1] = params_.colorHigh.y;
    gp.colorHigh[2] = params_.colorHigh.z; gp.colorHigh[3] = params_.colorHigh.w;
    gp.particleRadius = params_.particleRadius;
    gp.viewProj = viewProj_;

    // GPU 上で全計算 (Density → Force → Integrate) × substeps + PrepareInstances
    compute_.Dispatch(gp, params_.substeps);
}

// ============================================================
//  Draw — Object3d パイプラインで 1 DrawCall 描画
//         Object3DDraw() パス内で呼ぶこと
// ============================================================
void SphSimulator::Draw() {
    renderer_.Draw(compute_.GetInstanceBufferGPUAddr(),
                   static_cast<uint32_t>(compute_.GetParticleCount()));
}

// ============================================================
//  DrawImGui
// ============================================================
void SphSimulator::DrawImGui() {
#ifdef USE_IMGUI
    ImGui::Begin("SPH シミュレーター (GPU)");
    ImGui::Text("粒子数 : %d  [GPU Compute]", params_.particleCount);
    ImGui::Separator();
    ImGui::Text("[シミュレーション]");
    ImGui::DragFloat("影響半径",     &params_.smoothingRadius, 0.01f,  0.1f,  5.0f);
    ImGui::DragFloat("静止密度",     &params_.restDensity,     0.5f,   1.0f, 200.0f);
    ImGui::DragFloat("圧力剛性",     &params_.stiffness,       1.0f,   1.0f, 500.0f);
    ImGui::DragFloat("粘性係数",     &params_.viscosity,       0.1f,   0.0f,  50.0f);
    ImGui::DragFloat("粒子質量",     &params_.particleMass,    0.05f,  0.01f, 10.0f);
    ImGui::DragFloat("重力",         &params_.gravity,         0.1f, -30.0f,   0.0f);
    ImGui::DragFloat("壁反発係数",   &params_.restitution,     0.01f,  0.0f,   1.0f);
    ImGui::SliderInt("サブステップ", &params_.substeps,        1, 5);
    ImGui::Separator();
    ImGui::Text("[レンダリング]");
    ImGui::DragFloat("粒子半径",         &params_.particleRadius, 0.01f,  0.02f, 2.0f);
    ImGui::DragFloat("最大速度（色）",   &params_.speedMax,       0.1f,   0.1f, 50.0f);
    ImGui::ColorEdit4("低速カラー",      &params_.colorLow.x);
    ImGui::ColorEdit4("高速カラー",      &params_.colorHigh.x);
    ImGui::Separator();
    ImGui::Text("[境界ボックス]");
    ImGui::DragFloat3("最小座標", &params_.boundMin.x, 0.1f);
    ImGui::DragFloat3("最大座標", &params_.boundMax.x, 0.1f);
    ImGui::Separator();
    if (ImGui::Button("リセット")) Reset();
    ImGui::End();
#endif
}

// ============================================================
//  DrawBounds — バウンディングボックスの 12 辺をラインで描画
// ============================================================
void SphSimulator::DrawBounds(const TuboEngine::Math::Vector4& color) {
    auto* lm = TuboEngine::LineManager::GetInstance();
    using V3 = TuboEngine::Math::Vector3;
    const V3& mn = params_.boundMin;
    const V3& mx = params_.boundMax;

    // 底面 (Y = min)
    lm->DrawLine({mn.x, mn.y, mn.z}, {mx.x, mn.y, mn.z}, color);
    lm->DrawLine({mx.x, mn.y, mn.z}, {mx.x, mn.y, mx.z}, color);
    lm->DrawLine({mx.x, mn.y, mx.z}, {mn.x, mn.y, mx.z}, color);
    lm->DrawLine({mn.x, mn.y, mx.z}, {mn.x, mn.y, mn.z}, color);

    // 上面 (Y = max)
    lm->DrawLine({mn.x, mx.y, mn.z}, {mx.x, mx.y, mn.z}, color);
    lm->DrawLine({mx.x, mx.y, mn.z}, {mx.x, mx.y, mx.z}, color);
    lm->DrawLine({mx.x, mx.y, mx.z}, {mn.x, mx.y, mx.z}, color);
    lm->DrawLine({mn.x, mx.y, mx.z}, {mn.x, mx.y, mn.z}, color);

    // 縦辺 x4
    lm->DrawLine({mn.x, mn.y, mn.z}, {mn.x, mx.y, mn.z}, color);
    lm->DrawLine({mx.x, mn.y, mn.z}, {mx.x, mx.y, mn.z}, color);
    lm->DrawLine({mx.x, mn.y, mx.z}, {mx.x, mx.y, mx.z}, color);
    lm->DrawLine({mn.x, mn.y, mx.z}, {mn.x, mx.y, mx.z}, color);
}

// ============================================================
//  GenerateInitialParticles — CPU でグリッド配置を生成
// ============================================================
std::vector<SphParticle> SphSimulator::GenerateInitialParticles() const {
    std::vector<SphParticle> particles;
    particles.reserve(params_.particleCount);

    const float spacing = params_.smoothingRadius * 0.55f;
    const int   dim     = static_cast<int>(std::cbrt(float(params_.particleCount))) + 1;
    const TuboEngine::Math::Vector3 start = {
        params_.boundMin.x + spacing,
        (params_.boundMin.y + params_.boundMax.y) * 0.5f,
        params_.boundMin.z + spacing
    };

    int added = 0;
    for (int iz = 0; iz < dim && added < params_.particleCount; ++iz)
    for (int iy = 0; iy < dim && added < params_.particleCount; ++iy)
    for (int ix = 0; ix < dim && added < params_.particleCount; ++ix) {
        SphParticle p{};
        p.position = { start.x + ix * spacing,
                       start.y + iy * spacing,
                       start.z + iz * spacing };
        p.density  = params_.restDensity;
        particles.push_back(p);
        ++added;
    }
    return particles;
}

