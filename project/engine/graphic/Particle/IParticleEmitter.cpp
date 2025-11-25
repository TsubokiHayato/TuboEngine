#include "IParticleEmitter.h"
#include "Camera.h"
#include "SrvManager.h"
#include "TextureManager.h"
#include "ImGuiManager.h"
#include "MT_Matrix.h"
#include <numbers>
#include <cstring>
#undef min
#undef max

void IParticleEmitter::Initialize(const ParticlePreset& preset) {
    preset_ = preset;

    // 頂点形状生成
    vertices_.clear();
    BuildGeometry(vertices_);

    // テクスチャ読み込み（存在しない場合はフォールバック）
    if (!preset_.texture.empty()) {
        TextureManager::GetInstance()->LoadTexture(preset_.texture);
        textureSrvIndex_ = TextureManager::GetInstance()->GetSrvIndex(preset_.texture);
    }

    EnsureBuffers();
}

void IParticleEmitter::ResetInstances() {
    instanceCount_ = 0;
}

void IParticleEmitter::Emit(uint32_t count) {
    if (count == 0) return;
    // 最大インスタンス制限
    uint32_t remain = (preset_.maxInstances > instanceCount_)
        ? (preset_.maxInstances - instanceCount_)
        : 0;
    if (remain == 0) return;

    uint32_t emitCount = std::min(remain, count);
    for (uint32_t i = 0; i < emitCount; ++i) {
        PushParticle(GenerateParticle());
    }
}

void IParticleEmitter::Update(float dt, const Camera* camera) {
    if (dt <= 0.0f) return;

    // カメラ行列取得
    Matrix4x4 viewProj = MakeIdentity4x4();
    Matrix4x4 billboard = MakeIdentity4x4();

    if (camera) {
        // ViewProjection
        viewProj = camera->GetViewProjectionMatrix();

        // ビルボード用: カメラのワールド行列の回転成分 + 反転（必要なら）
        Matrix4x4 camWorld = camera->GetWorldMatrix();
        Matrix4x4 backToFront = MakeRotateYMatrix(std::numbers::pi_v<float>);
        billboard = Multiply(backToFront, camWorld);
        // 平行移動成分は無効化
        billboard.m[3][0] = 0.0f;
        billboard.m[3][1] = 0.0f;
        billboard.m[3][2] = 0.0f;
    }

    UpdateParticles(dt, viewProj, billboard);
}

void IParticleEmitter::Draw(ID3D12GraphicsCommandList* cmd) {
    if (!cmd || instanceCount_ == 0 || vertices_.empty()) return;
    if (!vb_ || !material_ || instancingSrvIndex_ < 0 || textureSrvIndex_ < 0) return;

    // 頂点バッファビュー
    D3D12_VERTEX_BUFFER_VIEW vbv{};
    vbv.BufferLocation = vb_->GetGPUVirtualAddress();
    vbv.SizeInBytes = static_cast<UINT>(sizeof(VertexData) * vertices_.size());
    vbv.StrideInBytes = sizeof(VertexData);

    cmd->IASetVertexBuffers(0, 1, &vbv);

    // ルートパラメータ（既存の Particle.cpp と同じスロットを使用）
    // 0: Material CBV, 1: Instancing SRV, 2: Texture SRV
    cmd->SetGraphicsRootConstantBufferView(0, material_->GetGPUVirtualAddress());
    cmd->SetGraphicsRootDescriptorTable(1, SrvManager::GetInstance()->GetGPUDescriptorHandle(instancingSrvIndex_));
    cmd->SetGraphicsRootDescriptorTable(2, SrvManager::GetInstance()->GetGPUDescriptorHandle(textureSrvIndex_));

    // インスタンシング描画
    cmd->DrawInstanced(static_cast<UINT>(vertices_.size()), instanceCount_, 0, 0);
}

void IParticleEmitter::DrawImGui() {
#ifdef USE_IMGUI
    if (ImGui::TreeNode(preset_.name.c_str())) {
        ImGui::Text("Instances: %u / %u", instanceCount_, preset_.maxInstances);
        ImGui::Checkbox("Billboard", &preset_.billboard);
        ImGui::DragInt("MaxInstances", reinterpret_cast<int*>(&preset_.maxInstances), 1, 1, 50000);
        ImGui::DragFloat("EmitRate", &preset_.emitRate, 0.2f, 0.0f, 2000.0f);
        ImGui::Checkbox("AutoEmit", &preset_.autoEmit);
        ImGui::DragInt("BurstCount", reinterpret_cast<int*>(&preset_.burstCount), 1, 1, 5000);

        ImGui::Separator();
        ImGui::DragFloat3("PosMin", &preset_.posMin.x, 0.01f);
        ImGui::DragFloat3("PosMax", &preset_.posMax.x, 0.01f);
        ImGui::DragFloat3("VelMin", &preset_.velMin.x, 0.01f);
        ImGui::DragFloat3("VelMax", &preset_.velMax.x, 0.01f);

        ImGui::DragFloat2("LifeRange", &preset_.lifeMin, 0.01f, 0.01f, 20.0f);
        ImGui::DragFloat3("ScaleStart", &preset_.scaleStart.x, 0.01f);
        ImGui::DragFloat3("ScaleEnd", &preset_.scaleEnd.x, 0.01f);
        ImGui::ColorEdit4("ColorStart", &preset_.colorStart.x);
        ImGui::ColorEdit4("ColorEnd", &preset_.colorEnd.x);

        ImGui::DragFloat3("Gravity", &preset_.gravity.x, 0.01f, -50.0f, 50.0f);
        ImGui::DragFloat("Drag", &preset_.drag, 0.001f, 0.0f, 2.0f);
        ImGui::DragFloat2("RotSpeedZ", &preset_.rotSpeedRangeZ.x, 0.01f, -10.0f, 10.0f);
        ImGui::DragFloat2("InitialRotZ", &preset_.initialRotRangeZ.x, 0.01f, -6.283f, 6.283f);

        ImGui::Checkbox("WorldSpace", &preset_.simulateInWorldSpace);
        ImGui::DragFloat3("EmitterPos", &preset_.emitterTransform.translate.x, 0.01f);

        ImGui::DragInt("BlendOverride", &preset_.blendModeOverride, -1, -1, 5);
        if (ImGui::Button("Emit Burst")) { Emit(preset_.burstCount); }
        ImGui::SameLine();
        if (ImGui::Button("Clear")) { particles_.clear(); instanceCount_ = 0; }

        ImGui::TreePop();
    }
#endif
}

// ---- 内部補助 ----

void IParticleEmitter::EnsureBuffers() {
    auto* dx = DirectXCommon::GetInstance();

    // 頂点バッファ
    if (!vb_ && !vertices_.empty()) {
        vb_ = dx->CreateBufferResource(sizeof(VertexData) * vertices_.size());
        void* mapped = nullptr;
        vb_->Map(0, nullptr, &mapped);
        std::memcpy(mapped, vertices_.data(), sizeof(VertexData) * vertices_.size());
        vb_->Unmap(0, nullptr);
    }

    // インスタンス用バッファ（構造化）
    if (!instancing_) {
        instancing_ = dx->CreateBufferResource(sizeof(ParticleForGPU) * preset_.maxInstances);
        instancing_->Map(0, nullptr, reinterpret_cast<void**>(&instancingPtr_));
        for (uint32_t i = 0; i < preset_.maxInstances; ++i) {
            instancingPtr_[i].WVP = MakeIdentity4x4();
            instancingPtr_[i].World = MakeIdentity4x4();
            instancingPtr_[i].color = {1,1,1,1};
        }
        // SRVを確保（既存の Particle.cpp と同様に +1 バイアスを維持）
        instancingSrvIndex_ = SrvManager::GetInstance()->Allocate() + 1;
        SrvManager::GetInstance()->CreateSRVForStructuredBuffer(
            instancingSrvIndex_, instancing_.Get(), preset_.maxInstances, sizeof(ParticleForGPU));
    }

    // マテリアル
    if (!material_) {
        material_ = dx->CreateBufferResource(sizeof(Material));
        material_->Map(0, nullptr, reinterpret_cast<void**>(&materialPtr_));
        materialPtr_->color = {1,1,1,1};
        materialPtr_->enableLighting = false; // パーティクルは基本 Unlit
        materialPtr_->uvTransform = MakeIdentity4x4();
    }
}

void IParticleEmitter::PushParticle(const ParticleInfo& p) {
    particles_.push_back(p);
    instanceCount_ = static_cast<uint32_t>(particles_.size());
}

void IParticleEmitter::UpdateParticles(float dt, const Matrix4x4& viewProj, const Matrix4x4& billboard) {
    for (auto it = particles_.begin(); it != particles_.end();) {
        it->currentTime += dt;
        if (it->currentTime >= it->lifeTime) {
            it = particles_.erase(it);
            continue;
        }

        float t = it->currentTime / it->lifeTime;
        // スケール補間
        it->transform.scale = {
            std::lerp(preset_.scaleStart.x, preset_.scaleEnd.x, t),
            std::lerp(preset_.scaleStart.y, preset_.scaleEnd.y, t),
            std::lerp(preset_.scaleStart.z, preset_.scaleEnd.z, t)
        };
        // カラー補間
        it->color = {
            std::lerp(preset_.colorStart.x, preset_.colorEnd.x, t),
            std::lerp(preset_.colorStart.y, preset_.colorEnd.y, t),
            std::lerp(preset_.colorStart.z, preset_.colorEnd.z, t),
            std::lerp(preset_.colorStart.w, preset_.colorEnd.w, t)
        };

        // 重力
        it->velocity.x += preset_.gravity.x * dt;
        it->velocity.y += preset_.gravity.y * dt;
        it->velocity.z += preset_.gravity.z * dt;

        // 減衰 (簡易: 速度 *= (1 - drag*dt))
        if (preset_.drag > 0.0f) {
            float d = std::clamp(1.0f - preset_.drag * dt, 0.0f, 1.0f);
            it->velocity.x *= d;
            it->velocity.y *= d;
            it->velocity.z *= d;
        }

        // 回転速度更新（rotSpeedZ がある前提）
        // it->transform.rotate.z += it->rotSpeedZ * dt; （構造体拡張後に追加）

        // 位置更新（ローカル/ワールド）
        Vector3 delta = {
            it->velocity.x * dt,
            it->velocity.y * dt,
            it->velocity.z * dt
        };
        if (preset_.simulateInWorldSpace) {
            it->transform.translate = it->transform.translate + delta;
        } else {
            // ローカル空間 → エミッター Transform を適用
            Vector3 base = preset_.emitterTransform.translate;
            it->transform.translate = (it->transform.translate + delta) + base;
        }

        ++it;
    }

    instanceCount_ = static_cast<uint32_t>(particles_.size());

    if (instancingPtr_ && instanceCount_ > 0) {
        uint32_t i = 0;
        for (auto& p : particles_) {
            Matrix4x4 local = MakeAffineMatrix(p.transform.scale, p.transform.rotate, p.transform.translate);
            Matrix4x4 world = preset_.billboard ? Multiply(billboard, local) : local;
            instancingPtr_[i].World = world;
            instancingPtr_[i].WVP = Multiply(world, viewProj);
            instancingPtr_[i].color = p.color;
            if (++i >= instanceCount_) break;
        }
    }
}