#include "SphSimulator.h"
#include "ImGuiManager.h"
#include <algorithm>
#include <cmath>
#include <cstring>
#include <numbers>
#undef min
#undef max

// ============================================================
//  定数
// ============================================================
static constexpr float kPi = std::numbers::pi_v<float>;

// ============================================================
//  Initialize
// ============================================================
void SphSimulator::Initialize(const Params& params, const std::string& texture) {
    params_      = params;
    texturePath_ = texture;

    // テクスチャロード
    TuboEngine::TextureManager::GetInstance()->LoadTexture(texturePath_);
    textureSrvIndex_ = TuboEngine::TextureManager::GetInstance()->GetSrvIndex(texturePath_);

    // ビルボードクワッド頂点生成
    BuildGeometry();

    // DirectX12 リソース確保
    EnsureBuffers();

    // 行列初期化
    viewProj_  = TuboEngine::Math::MakeIdentity4x4();
    billboard_ = TuboEngine::Math::MakeIdentity4x4();

    // 粒子初期配置
    Reset();
}

// ============================================================
//  Finalize
// ============================================================
void SphSimulator::Finalize() {
    if (material_ && materialPtr_) {
        material_->Unmap(0, nullptr);
        materialPtr_ = nullptr;
    }
    if (instancing_ && instancingPtr_) {
        instancing_->Unmap(0, nullptr);
        instancingPtr_ = nullptr;
    }
    vb_.Reset();
    material_.Reset();
    instancing_.Reset();
    particles_.clear();
    buffersReady_ = false;
}

// ============================================================
//  Reset  — 粒子を格子状に初期配置
// ============================================================
void SphSimulator::Reset() {
    particles_.clear();
    particles_.reserve(params_.particleCount);

    // 境界ボックス上半分に格子配置
    const float spacing = params_.smoothingRadius * 0.55f;
    const int   dim     = static_cast<int>(std::cbrt(static_cast<float>(params_.particleCount))) + 1;

    const TuboEngine::Math::Vector3 start = {
        params_.boundMin.x + spacing,
        (params_.boundMin.y + params_.boundMax.y) * 0.5f,
        params_.boundMin.z + spacing
    };

    int added = 0;
    for (int iz = 0; iz < dim && added < params_.particleCount; ++iz) {
        for (int iy = 0; iy < dim && added < params_.particleCount; ++iy) {
            for (int ix = 0; ix < dim && added < params_.particleCount; ++ix) {
                SphParticle p{};
                p.position = {
                    start.x + ix * spacing,
                    start.y + iy * spacing,
                    start.z + iz * spacing
                };
                p.density  = params_.restDensity;
                particles_.push_back(p);
                ++added;
            }
        }
    }
}

// ============================================================
//  Update  — SPH 計算 + GPU バッファ更新
// ============================================================
void SphSimulator::Update(float dt, const TuboEngine::Camera* camera) {
    if (particles_.empty()) return;

    // サブステップ (安定化)
    constexpr int kSubsteps = 3;
    const float   subDt     = dt / static_cast<float>(kSubsteps);
    for (int s = 0; s < kSubsteps; ++s) {
        ComputeDensityPressure();
        ComputeForces();
        Integrate(subDt);
        HandleBoundaries();
    }

    // ビュー行列・ビルボード行列を更新
    if (camera) {
        viewProj_ = camera->GetViewProjectionMatrix();

        // ビルボード = カメラワールド行列の回転成分（平行移動除く）
        TuboEngine::Math::Matrix4x4 camWorld  = camera->GetWorldMatrix();
        TuboEngine::Math::Matrix4x4 backToFront =
            TuboEngine::Math::MakeRotateYMatrix(std::numbers::pi_v<float>);
        billboard_ = Multiply(backToFront, camWorld);
        billboard_.m[3][0] = 0.0f;
        billboard_.m[3][1] = 0.0f;
        billboard_.m[3][2] = 0.0f;
    }

    UploadInstances(viewProj_, billboard_);
}

// ============================================================
//  Draw  — ParticlePSO が有効な描画パスで呼ぶ
// ============================================================
void SphSimulator::Draw() {
    if (!buffersReady_ || instanceCount_ == 0 || vertices_.empty()) return;
    if (!vb_ || !material_ || instancingSrvIndex_ < 0 || textureSrvIndex_ < 0) return;

    auto* cmd = TuboEngine::DirectXCommon::GetInstance()->GetCommandList().Get();

    D3D12_VERTEX_BUFFER_VIEW vbv{};
    vbv.BufferLocation = vb_->GetGPUVirtualAddress();
    vbv.SizeInBytes    = static_cast<UINT>(sizeof(TuboEngine::VertexData) * vertices_.size());
    vbv.StrideInBytes  = sizeof(TuboEngine::VertexData);
    cmd->IASetVertexBuffers(0, 1, &vbv);

    // root[0]: マテリアル CBV
    cmd->SetGraphicsRootConstantBufferView(0, material_->GetGPUVirtualAddress());
    // root[1]: インスタンシング SRV
    cmd->SetGraphicsRootDescriptorTable(
        1, TuboEngine::SrvManager::GetInstance()->GetGPUDescriptorHandle(instancingSrvIndex_));
    // root[2]: テクスチャ SRV
    cmd->SetGraphicsRootDescriptorTable(
        2, TuboEngine::SrvManager::GetInstance()->GetGPUDescriptorHandle(textureSrvIndex_));

    cmd->DrawInstanced(static_cast<UINT>(vertices_.size()), instanceCount_, 0, 0);
}

// ============================================================
//  DrawImGui
// ============================================================
void SphSimulator::DrawImGui() {
#ifdef USE_IMGUI
    ImGui::Begin("SPH Simulator");

    ImGui::Text("Particles : %u / %d", instanceCount_, params_.particleCount);

    ImGui::Separator();
    ImGui::Text("[Simulation]");
    ImGui::DragFloat("Smoothing Radius",  &params_.smoothingRadius, 0.01f, 0.1f, 5.0f);
    ImGui::DragFloat("Rest Density",      &params_.restDensity,     5.0f,  10.0f, 5000.0f);
    ImGui::DragFloat("Stiffness",         &params_.stiffness,       1.0f,  1.0f,  2000.0f);
    ImGui::DragFloat("Viscosity",         &params_.viscosity,       0.001f,0.0f,  2.0f);
    ImGui::DragFloat("Particle Mass",     &params_.particleMass,    0.001f,0.001f,1.0f);
    ImGui::DragFloat("Gravity",           &params_.gravity,         0.1f, -50.0f, 0.0f);
    ImGui::DragFloat("Restitution",       &params_.restitution,     0.01f, 0.0f,  1.0f);

    ImGui::Separator();
    ImGui::Text("[Rendering]");
    ImGui::DragFloat("Visual Radius",     &params_.particleRadius,  0.005f,0.01f, 1.0f);
    ImGui::ColorEdit4("Color Low Speed",  &params_.colorLow.x);
    ImGui::ColorEdit4("Color High Speed", &params_.colorHigh.x);

    ImGui::Separator();
    ImGui::Text("[Boundary]");
    ImGui::DragFloat3("Bound Min", &params_.boundMin.x, 0.1f);
    ImGui::DragFloat3("Bound Max", &params_.boundMax.x, 0.1f);

    ImGui::Separator();
    if (ImGui::Button("Reset Particles")) {
        Reset();
    }

    ImGui::End();
#endif
}

// ============================================================
//  SPH : 密度・圧力計算
//  ρᵢ = Σⱼ mⱼ W_poly6(|rᵢ-rⱼ|², h)
//  pᵢ = k (ρᵢ - ρ₀)
// ============================================================
void SphSimulator::ComputeDensityPressure() {
    const float h  = params_.smoothingRadius;
    const float m  = params_.particleMass;
    const float k  = params_.stiffness;
    const float r0 = params_.restDensity;

    for (auto& pi : particles_) {
        pi.density = 0.0f;
        for (const auto& pj : particles_) {
            const TuboEngine::Math::Vector3 rij = pi.position - pj.position;
            const float r2 = rij.LengthSquared();
            pi.density += m * KernelPoly6(r2, h);
        }
        pi.density  = std::max(pi.density, 1e-6f);
        pi.pressure = k * (pi.density - r0);
    }
}

// ============================================================
//  SPH : 力の計算
//  圧力: f_press = -m Σⱼ (pᵢ+pⱼ)/(2ρⱼ) ∇W_spiky
//  粘性: f_visc  = μ m Σⱼ (vⱼ-vᵢ)/ρⱼ  ∇²W_visc
//  重力: f_grav  = ρᵢ g ĵ
// ============================================================
void SphSimulator::ComputeForces() {
    const float h  = params_.smoothingRadius;
    const float m  = params_.particleMass;
    const float mu = params_.viscosity;

    const size_t n = particles_.size();
    for (size_t i = 0; i < n; ++i) {
        auto& pi = particles_[i];
        TuboEngine::Math::Vector3 fPressure  = {0.0f, 0.0f, 0.0f};
        TuboEngine::Math::Vector3 fViscosity = {0.0f, 0.0f, 0.0f};

        for (size_t j = 0; j < n; ++j) {
            if (i == j) continue;
            const auto& pj = particles_[j];

            const TuboEngine::Math::Vector3 rij = pi.position - pj.position;
            const float r = rij.Length();
            if (r >= h || r < 1e-6f) continue;

            // 圧力項
            const float pTerm = (pi.pressure + pj.pressure) / (2.0f * pj.density);
            const TuboEngine::Math::Vector3 grad = KernelSpikyGrad(rij, r, h);
            fPressure += grad * (-m * pTerm);

            // 粘性項
            const TuboEngine::Math::Vector3 velDiff = pj.velocity - pi.velocity;
            const float lap = KernelViscLap(r, h);
            fViscosity += velDiff * (mu * m / pj.density * lap);
        }

        // 重力
        const TuboEngine::Math::Vector3 fGravity = {
            0.0f,
            params_.gravity * pi.density,
            0.0f
        };

        pi.force = fPressure + fViscosity + fGravity;
    }
}

// ============================================================
//  SPH : セミインプリシット Euler 積分
//  aᵢ = fᵢ / ρᵢ
//  vᵢ += aᵢ dt
//  xᵢ += vᵢ dt
// ============================================================
void SphSimulator::Integrate(float dt) {
    for (auto& p : particles_) {
        const TuboEngine::Math::Vector3 accel = p.force / p.density;
        p.velocity += accel * dt;
        p.position += p.velocity * dt;
    }
}

// ============================================================
//  SPH : 境界条件 (AABB 反射)
// ============================================================
void SphSimulator::HandleBoundaries() {
    const auto& mn = params_.boundMin;
    const auto& mx = params_.boundMax;
    const float e  = params_.restitution;

    for (auto& p : particles_) {
        auto& pos = p.position;
        auto& vel = p.velocity;

        if (pos.x < mn.x) { pos.x = mn.x; vel.x =  std::abs(vel.x) * e; }
        if (pos.x > mx.x) { pos.x = mx.x; vel.x = -std::abs(vel.x) * e; }
        if (pos.y < mn.y) { pos.y = mn.y; vel.y =  std::abs(vel.y) * e; }
        if (pos.y > mx.y) { pos.y = mx.y; vel.y = -std::abs(vel.y) * e; }
        if (pos.z < mn.z) { pos.z = mn.z; vel.z =  std::abs(vel.z) * e; }
        if (pos.z > mx.z) { pos.z = mx.z; vel.z = -std::abs(vel.z) * e; }
    }
}

// ============================================================
//  SPH カーネル : Poly6  (密度用, r² を渡す)
//  W(r,h) = 315/(64π h⁹) (h²-r²)³
// ============================================================
float SphSimulator::KernelPoly6(float r2, float h) const {
    if (r2 >= h * h) return 0.0f;
    const float d = h * h - r2;
    const float coeff = 315.0f / (64.0f * kPi * std::pow(h, 9.0f));
    return coeff * d * d * d;
}

// ============================================================
//  SPH カーネル : Spiky 勾配  (圧力用)
//  ∇W = -45/(π h⁶) (h-r)² (rij/r)
// ============================================================
TuboEngine::Math::Vector3 SphSimulator::KernelSpikyGrad(
    const TuboEngine::Math::Vector3& rij, float r, float h) const
{
    if (r >= h || r < 1e-6f) return {0.0f, 0.0f, 0.0f};
    const float d     = h - r;
    const float coeff = -(45.0f / (kPi * std::pow(h, 6.0f))) * d * d / r;
    return rij * coeff;
}

// ============================================================
//  SPH カーネル : Viscosity ラプラシアン (粘性用)
//  ∇²W = 45/(π h⁶) (h-r)
// ============================================================
float SphSimulator::KernelViscLap(float r, float h) const {
    if (r >= h) return 0.0f;
    return (45.0f / (kPi * std::pow(h, 6.0f))) * (h - r);
}

// ============================================================
//  BuildGeometry  — ビルボード用ユニットクワッド (IParticleEmitter と同形式)
// ============================================================
void SphSimulator::BuildGeometry() {
    vertices_.clear();
    TuboEngine::VertexData v0{}, v1{}, v2{}, v3{};
    v0.position = { 1.0f,  1.0f, 0.0f, 1.0f}; v0.texcoord = {0.0f, 0.0f}; v0.normal = {0,0,1};
    v1.position = {-1.0f,  1.0f, 0.0f, 1.0f}; v1.texcoord = {1.0f, 0.0f}; v1.normal = {0,0,1};
    v2.position = { 1.0f, -1.0f, 0.0f, 1.0f}; v2.texcoord = {0.0f, 1.0f}; v2.normal = {0,0,1};
    v3.position = {-1.0f, -1.0f, 0.0f, 1.0f}; v3.texcoord = {1.0f, 1.0f}; v3.normal = {0,0,1};
    vertices_.push_back(v0); vertices_.push_back(v1); vertices_.push_back(v2);
    vertices_.push_back(v2); vertices_.push_back(v1); vertices_.push_back(v3);
}

// ============================================================
//  EnsureBuffers  — DirectX12 リソース確保 (初期化時に 1 回)
// ============================================================
void SphSimulator::EnsureBuffers() {
    auto* dx = TuboEngine::DirectXCommon::GetInstance();

    // 頂点バッファ
    if (!vb_ && !vertices_.empty()) {
        vb_ = dx->CreateBufferResource(sizeof(TuboEngine::VertexData) * vertices_.size());
        void* mapped = nullptr;
        vb_->Map(0, nullptr, &mapped);
        std::memcpy(mapped, vertices_.data(), sizeof(TuboEngine::VertexData) * vertices_.size());
        vb_->Unmap(0, nullptr);
    }

    // インスタンシング StructuredBuffer
    if (!instancing_) {
        const uint32_t count = static_cast<uint32_t>(params_.particleCount);
        instancing_     = dx->CreateBufferResource(sizeof(SphGPUInstance) * count);
        allocatedCount_ = count;
        instancing_->Map(0, nullptr, reinterpret_cast<void**>(&instancingPtr_));
        for (uint32_t i = 0; i < count; ++i) {
            instancingPtr_[i].WVP   = TuboEngine::Math::MakeIdentity4x4();
            instancingPtr_[i].World = TuboEngine::Math::MakeIdentity4x4();
            instancingPtr_[i].color = {1,1,1,1};
        }
        instancingSrvIndex_ = TuboEngine::SrvManager::GetInstance()->Allocate();
        TuboEngine::SrvManager::GetInstance()->CreateSRVForStructuredBuffer(
            instancingSrvIndex_, instancing_.Get(), count, sizeof(SphGPUInstance));
    }

    // マテリアル定数バッファ
    if (!material_) {
        material_ = dx->CreateBufferResource(sizeof(TuboEngine::Material));
        material_->Map(0, nullptr, reinterpret_cast<void**>(&materialPtr_));
        materialPtr_->color          = {1.0f, 1.0f, 1.0f, 1.0f};
        materialPtr_->enableLighting = false;
        materialPtr_->uvTransform    = TuboEngine::Math::MakeIdentity4x4();
    }

    buffersReady_ = true;
}

// ============================================================
//  UploadInstances  — CPU → GPU にインスタンスデータを書き込む
//  色は粒子速度で補間 (低速 → colorLow, 高速 → colorHigh)
// ============================================================
void SphSimulator::UploadInstances(
    const TuboEngine::Math::Matrix4x4& viewProj,
    const TuboEngine::Math::Matrix4x4& billboard)
{
    if (!instancingPtr_ || particles_.empty()) return;

    instanceCount_ = 0;
    const float speedMax = 5.0f;  // 色が最大になる速度 (m/s)

    for (const auto& p : particles_) {
        if (instanceCount_ >= allocatedCount_) break;

        // 速度で色補間
        const float t = std::clamp(p.velocity.Length() / speedMax, 0.0f, 1.0f);
        const TuboEngine::Math::Vector4 color = {
            std::lerp(params_.colorLow.x, params_.colorHigh.x, t),
            std::lerp(params_.colorLow.y, params_.colorHigh.y, t),
            std::lerp(params_.colorLow.z, params_.colorHigh.z, t),
            std::lerp(params_.colorLow.w, params_.colorHigh.w, t),
        };

        // ワールド行列 = ビルボード × アフィン変換
        const TuboEngine::Math::Vector3 scale = {
            params_.particleRadius,
            params_.particleRadius,
            params_.particleRadius
        };
        const TuboEngine::Math::Matrix4x4 local =
            TuboEngine::Math::MakeAffineMatrix(scale, {0.0f,0.0f,0.0f}, p.position);
        const TuboEngine::Math::Matrix4x4 world = Multiply(billboard, local);

        instancingPtr_[instanceCount_].World = world;
        instancingPtr_[instanceCount_].WVP   = Multiply(world, viewProj);
        instancingPtr_[instanceCount_].color  = color;
        ++instanceCount_;
    }
}
