#include "SphSimulator.h"
#include "ImGuiManager.h"
#include "Input.h"
#include "WinApp.h"
#include <algorithm>
#include <cmath>
#include <cstring>
#include <numbers>
#undef min
#undef max

static constexpr float kPi = std::numbers::pi_v<float>;

// 行ベクトル × 行優先行列 (このエンジンの規約: pos * matrix)
static TuboEngine::Math::Vector4 MulRowVec4(const TuboEngine::Math::Vector4& v,
                                            const TuboEngine::Math::Matrix4x4& m) {
    return {
        v.x * m.m[0][0] + v.y * m.m[1][0] + v.z * m.m[2][0] + v.w * m.m[3][0],
        v.x * m.m[0][1] + v.y * m.m[1][1] + v.z * m.m[2][1] + v.w * m.m[3][1],
        v.x * m.m[0][2] + v.y * m.m[1][2] + v.z * m.m[2][2] + v.w * m.m[3][2],
        v.x * m.m[0][3] + v.y * m.m[1][3] + v.z * m.m[2][3] + v.w * m.m[3][3],
    };
}

// ============================================================
//  Initialize
// ============================================================
void SphSimulator::Initialize(const Params& params, TuboEngine::Camera* camera,
                                const std::string& modelPath, const std::string& texture)
{
    params_ = params;

    viewProj_ = TuboEngine::Math::MakeIdentity4x4();

    // GPU Compute パイプライン (セルサイズ = smoothingRadius で空間ハッシュ構築)
    compute_.Initialize(params_.particleCount, params_.boundMin, params_.boundMax,
                        params_.smoothingRadius);

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

    // マウスドラッグで外力点を操作
    UpdateMouseForce();

    // 再生制御: 一時停止中は dt=0 (描画行列は更新される)、再生速度でスケール、
    // コマ送りは次の1フレームだけ通常 dt で進める
    float simDt = dt * timeScale_;
    if (paused_ && !stepOnce_) simDt = 0.0f;
    stepOnce_ = false;

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
    gp.dt            = simDt / float(params_.substeps);
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
    gp.xsphCoeff      = params_.xsphCoeff;
    gp.extForcePosX     = params_.extForcePos.x;
    gp.extForcePosY     = params_.extForcePos.y;
    gp.extForcePosZ     = params_.extForcePos.z;
    gp.extForceRadius   = params_.extForceRadius;
    gp.extForceStrength = params_.extForceStrength;
    gp.extForceActive   = params_.extForceActive ? 1 : 0;
    gp.surfaceTension   = params_.surfaceTension;
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
#ifdef USE_IMGUI
// (?) アイコンにマウスを乗せると説明を表示するヘルパー
static void HelpMarker(const char* desc) {
    ImGui::SameLine();
    ImGui::TextDisabled("(?)");
    if (ImGui::IsItemHovered()) {
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 25.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}
#endif

void SphSimulator::DrawImGui() {
#ifdef USE_IMGUI
    ImGui::Begin("SPH シミュレーター (GPU)");
    ImGui::Text("粒子数 : %d  [GPU Compute]", params_.particleCount);

    // ---- プリセット (ワンクリックで質感を切替) ----
    ImGui::Separator();
    ImGui::Text("プリセット");
    if (ImGui::Button("水"))       ApplyPreset(Preset::Water);
    ImGui::SameLine();
    if (ImGui::Button("ハチミツ")) ApplyPreset(Preset::Honey);
    ImGui::SameLine();
    if (ImGui::Button("スライム")) ApplyPreset(Preset::Slime);

    // ---- 再生コントロール ----
    ImGui::Separator();
    ImGui::Text("再生");
    if (paused_) { if (ImGui::Button("再生"))      paused_ = false; }
    else         { if (ImGui::Button("一時停止"))  paused_ = true;  }
    ImGui::SameLine();
    if (ImGui::Button("コマ送り")) { stepOnce_ = true; paused_ = true; }
    ImGui::SameLine();
    if (ImGui::Button("リセット")) Reset();
    ImGui::DragFloat("再生速度", &timeScale_, 0.01f, 0.0f, 2.0f);
    HelpMarker("シミュレーションの速さ。1.0=等速、0.5=スロー、0=停止");

    // ---- シミュレーション ----
    if (ImGui::CollapsingHeader("シミュレーション", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::DragFloat("影響半径",   &params_.smoothingRadius, 0.01f, 0.1f, 5.0f);
        HelpMarker("粒子が影響し合う距離。大きいほど滑らかだが重い。\n注意: 初期値より大きくすると空間ハッシュが近傍を取りこぼす");
        ImGui::DragFloat("静止密度",   &params_.restDensity, 0.1f, 1.0f, 30.0f);
        HelpMarker("流体が保とうとする密度。低いとスカスカ、高いとギュッと詰まる");
        ImGui::DragFloat("圧力剛性",   &params_.stiffness, 1.0f, 1.0f, 500.0f);
        HelpMarker("圧力の強さ。高いほど非圧縮(硬い水)。高すぎると爆発する");
        ImGui::DragFloat("粘性係数",   &params_.viscosity, 0.05f, 0.0f, 50.0f);
        HelpMarker("ねばり。低い=サラサラ(水)、高い=とろとろ(ハチミツ)");
        ImGui::DragFloat("XSPH補正",   &params_.xsphCoeff, 0.01f, 0.0f, 1.0f);
        HelpMarker("粒子の足並みを揃える。高いほど一体感のある流れになる");
        ImGui::DragFloat("表面張力",   &params_.surfaceTension, 0.05f, 0.0f, 20.0f);
        HelpMarker("水面のまとまり。高いほど水滴が丸まり、しずくになりやすい");
        ImGui::DragFloat("粒子質量",   &params_.particleMass, 0.05f, 0.01f, 10.0f);
        ImGui::DragFloat("重力",       &params_.gravity, 0.1f, -30.0f, 0.0f);
        ImGui::DragFloat("壁反発係数", &params_.restitution, 0.01f, 0.0f, 1.0f);
        HelpMarker("壁での跳ね返り。0=跳ねない(液体)、1=完全反射");
        ImGui::SliderInt("サブステップ", &params_.substeps, 1, 5);
        HelpMarker("1フレームの分割数。多いほど安定だが重い");
    }

    // ---- レンダリング ----
    if (ImGui::CollapsingHeader("レンダリング")) {
        ImGui::DragFloat("粒子半径",       &params_.particleRadius, 0.01f, 0.02f, 2.0f);
        ImGui::DragFloat("最大速度（色）", &params_.speedMax, 0.1f, 0.1f, 50.0f);
        HelpMarker("この速度に達すると高速カラーになる");
        ImGui::ColorEdit4("低速カラー",    &params_.colorLow.x);
        ImGui::ColorEdit4("高速カラー",    &params_.colorHigh.x);
    }

    // ---- 境界ボックス ----
    if (ImGui::CollapsingHeader("境界ボックス")) {
        ImGui::DragFloat3("最小座標", &params_.boundMin.x, 0.1f);
        ImGui::DragFloat3("最大座標", &params_.boundMax.x, 0.1f);
        HelpMarker("変更後はリセットで粒子を再配置すると綺麗");
    }

    // ---- 外力 ----
    if (ImGui::CollapsingHeader("外力", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Checkbox("マウスで操作", &mouseForceEnabled_);
        HelpMarker("ON にして画面を左ドラッグすると、その位置の流体を押せる");
        ImGui::Checkbox("外力 有効",   &params_.extForceActive);
        ImGui::DragFloat3("外力 位置", &params_.extForcePos.x, 0.1f);
        ImGui::DragFloat("外力 半径",  &params_.extForceRadius, 0.1f, 0.1f, 20.0f);
        ImGui::DragFloat("外力 強さ",  &params_.extForceStrength, 1.0f, -300.0f, 300.0f);
        HelpMarker("正=押し出す(噴水)、負=引き寄せる(集める)");
    }

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

    // 外力点の可視化 (有効時、力点に影響半径サイズの十字を描画)
    if (params_.extForceActive) {
        const V3&   c  = params_.extForcePos;
        const float r  = params_.extForceRadius;
        const TuboEngine::Math::Vector4 fc = {1.0f, 0.4f, 0.1f, 1.0f}; // オレンジ
        lm->DrawLine({c.x - r, c.y, c.z}, {c.x + r, c.y, c.z}, fc);
        lm->DrawLine({c.x, c.y - r, c.z}, {c.x, c.y + r, c.z}, fc);
        lm->DrawLine({c.x, c.y, c.z - r}, {c.x, c.y, c.z + r}, fc);
    }
}

// ============================================================
//  GenerateInitialParticles — CPU でグリッド配置を生成
// ============================================================
std::vector<SphParticle> SphSimulator::GenerateInitialParticles() const {
    std::vector<SphParticle> particles;
    particles.reserve(params_.particleCount);

    const float spacing    = params_.smoothingRadius * 0.55f;
    const int   dim        = static_cast<int>(std::cbrt(float(params_.particleCount))) + 1;
    const float gridExtent = (dim - 1) * spacing;

    // グリッドをボックス中央(XZ)・底面(Y)に配置
    // ボックス外に粒子が出ないよう中央揃え
    const TuboEngine::Math::Vector3 start = {
        (params_.boundMin.x + params_.boundMax.x) * 0.5f - gridExtent * 0.5f,
        params_.boundMin.y + spacing,   // 底面から積み上げる (重力で落下するので自然)
        (params_.boundMin.z + params_.boundMax.z) * 0.5f - gridExtent * 0.5f
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

// ============================================================
//  ApplyPreset — 質感プリセットを適用 (粒子数/境界は変えない)
// ============================================================
void SphSimulator::ApplyPreset(Preset preset) {
    switch (preset) {
    case Preset::Water:   // サラサラの水
        params_.viscosity      = 1.0f;
        params_.stiffness      = 200.0f;
        params_.xsphCoeff      = 0.15f;
        params_.restitution    = 0.02f;
        params_.surfaceTension = 2.0f;
        break;
    case Preset::Honey:   // とろとろのハチミツ
        params_.viscosity      = 40.0f;
        params_.stiffness      = 150.0f;
        params_.xsphCoeff      = 0.30f;
        params_.restitution    = 0.0f;
        params_.surfaceTension = 1.0f;
        break;
    case Preset::Slime:   // ぷるぷるのスライム (高めの表面張力で丸まる)
        params_.viscosity      = 15.0f;
        params_.stiffness      = 250.0f;
        params_.xsphCoeff      = 0.45f;
        params_.restitution    = 0.0f;
        params_.surfaceTension = 8.0f;
        break;
    }
}

// ============================================================
//  UpdateMouseForce — マウス左ドラッグで外力点を操作
//  スクリーン座標をボックス中心の深度平面へアンプロジェクトする
// ============================================================
void SphSimulator::UpdateMouseForce() {
    if (!mouseForceEnabled_) {
        if (mouseDriving_) { params_.extForceActive = false; mouseDriving_ = false; }
        return;
    }

    auto* input = TuboEngine::Input::GetInstance();

#ifdef USE_IMGUI
    // ImGui ウィンドウ操作中はマウス外力を無効化
    if (ImGui::GetIO().WantCaptureMouse) {
        if (mouseDriving_) { params_.extForceActive = false; mouseDriving_ = false; }
        return;
    }
#endif

    if (input->IsPressMouse(0)) {
        const float W = static_cast<float>(TuboEngine::WinApp::GetInstance()->GetClientWidth());
        const float H = static_cast<float>(TuboEngine::WinApp::GetInstance()->GetClientHeight());
        const auto  mp = input->GetMousePosition();

        // スクリーン → NDC
        const float ndcX = 2.0f * mp.x / W - 1.0f;
        const float ndcY = 1.0f - 2.0f * mp.y / H;

        // ボックス中心の深度 (NDC z) を基準面に使う
        const TuboEngine::Math::Vector3 center = {
            (params_.boundMin.x + params_.boundMax.x) * 0.5f,
            (params_.boundMin.y + params_.boundMax.y) * 0.5f,
            (params_.boundMin.z + params_.boundMax.z) * 0.5f
        };
        const TuboEngine::Math::Vector4 cClip = MulRowVec4({center.x, center.y, center.z, 1.0f}, viewProj_);
        const float ndcZ = (std::abs(cClip.w) > 1e-6f) ? cClip.z / cClip.w : 0.5f;

        // NDC → ワールド (逆 ViewProjection)
        const TuboEngine::Math::Matrix4x4 invVP = TuboEngine::Math::Inverse(viewProj_);
        const TuboEngine::Math::Vector4 wp = MulRowVec4({ndcX, ndcY, ndcZ, 1.0f}, invVP);
        if (std::abs(wp.w) > 1e-6f) {
            params_.extForcePos    = { wp.x / wp.w, wp.y / wp.w, wp.z / wp.w };
            params_.extForceActive = true;
            mouseDriving_          = true;
        }
    } else if (mouseDriving_) {
        // ボタンを離したらマウス駆動の外力をOFF
        params_.extForceActive = false;
        mouseDriving_          = false;
    }
}

