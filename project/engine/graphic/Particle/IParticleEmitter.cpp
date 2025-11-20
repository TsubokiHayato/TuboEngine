#include "IParticleEmitter.h"
#include "Camera.h"
#include "SrvManager.h"
#include "TextureManager.h"
#include"ImGuiManager.h"
#include "MT_Matrix.h"
#undef min
#undef max


// 最低限の安全チェック付き簡易実装

void IParticleEmitter::Initialize(const ParticlePreset& preset) {
    preset_ = preset;

    // 頂点形状生成
    vertices_.clear();
    BuildGeometry(vertices_);

    EnsureBuffers();

    // テクスチャ読み込み（存在しない場合はフォールバック）
    if (!preset_.texture.empty()) {
       TextureManager::GetInstance()->LoadTexture(preset_.texture);
		textureSrvIndex_ = TextureManager::GetInstance()->GetSrvIndex(preset_.texture);
    }
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
        auto p = GenerateParticle();
        PushParticle(p);
    }
}

void IParticleEmitter::Update(float dt, const Camera* camera) {
    if (dt <= 0.0f) return;

    // カメラ行列取得
    Matrix4x4 viewProj{};
    Matrix4x4 billboard{};
    if (camera) {
        viewProj = camera->GetViewProjectionMatrix();
        // ビルボード用の簡易行列（本来は回転成分だけカメラに合わせる）
        billboard = camera->GetViewMatrix();
    }
    UpdateParticles(dt, viewProj, billboard);
}

void IParticleEmitter::Draw(ID3D12GraphicsCommandList* cmd) {
    if (!cmd || instanceCount_ == 0 || vertices_.empty()) return;

    // 必要なら動的インスタンシング更新 (既に UpdateParticles 内で instancingPtr_ 書き込み済み想定)
    // ここで VB/インスタンスバッファ/マテリアル/テクスチャをセットして DrawCall を行う処理を書く
    // 具体的なレンダリングラッパが不明のため空にしてあります。
}

void IParticleEmitter::DrawImGui() {
#ifdef USE_IMGUI
    if (ImGui::TreeNode(preset_.name.c_str())) {
        ImGui::Text("Instances: %u / %u", instanceCount_, preset_.maxInstances);
        ImGui::Checkbox("Billboard", &preset_.billboard);
        ImGui::DragInt("MaxInstances", reinterpret_cast<int*>(&preset_.maxInstances), 1, 1, 100000);
        ImGui::DragFloat2("Life", &preset_.lifeMin, 0.01f, 0.0f, 30.0f);
        ImGui::DragFloat3("PosMin", &preset_.posMin.x, 0.01f);
        ImGui::DragFloat3("PosMax", &preset_.posMax.x, 0.01f);
        ImGui::DragFloat3("VelMin", &preset_.velMin.x, 0.01f);
        ImGui::DragFloat3("VelMax", &preset_.velMax.x, 0.01f);
        ImGui::DragFloat3("ScaleMin", &preset_.scaleMin.x, 0.01f);
        ImGui::DragFloat3("ScaleMax", &preset_.scaleMax.x, 0.01f);
        ImGui::ColorEdit4("ColMin", &preset_.colMin.x);
        ImGui::ColorEdit4("ColMax", &preset_.colMax.x);
        if (ImGui::Button("Emit Burst")) {
            Emit(preset_.burstCount);
        }
        ImGui::TreePop();
    }
#endif
}

// ---- 内部補助 ----

void IParticleEmitter::EnsureBuffers() {
    // 頂点バッファ/インスタンスバッファ/マテリアルリソース確保
    // 実装詳細が不明のため最小限の初期化だけ（存在チェックのみ）
    if (!vb_ && !vertices_.empty()) {
        // ここで頂点バッファ生成処理を書く
    }
    if (!instancing_) {
        // インスタンス用バッファ生成（最大 maxInstances 分）
    }
    if (!material_) {
        // マテリアルリソース生成
    }
}

void IParticleEmitter::PushParticle(const ParticleInfo& p) {
    particles_.push_back(p);
    instanceCount_ = static_cast<uint32_t>(particles_.size());
}

void IParticleEmitter::UpdateParticles(float dt, const Matrix4x4& viewProj, const Matrix4x4& billboard) {
    // 寿命更新 & 位置更新
    for (auto it = particles_.begin(); it != particles_.end(); ) {
        it->currentTime += dt;
        if (it->currentTime >= it->lifeTime) {
            it = particles_.erase(it);
            continue;
        }
        // 単純な速度更新
        it->transform.translate.x += it->velocity.x * dt;
        it->transform.translate.y += it->velocity.y * dt;
        it->transform.translate.z += it->velocity.z * dt;
        ++it;
    }
    instanceCount_ = static_cast<uint32_t>(particles_.size());

    // GPUインスタンスバッファ書き込み（簡易）
    if (instancingPtr_ && instanceCount_ > 0) {
        uint32_t i = 0;
        for (auto& p : particles_) {
            // 本来は行列計算 (World, WVP) を行う
            instancingPtr_[i].World = MakeAffineMatrix(p.transform.scale, p.transform.rotate, p.transform.translate);
            instancingPtr_[i].WVP = instancingPtr_[i].World; // 簡易（本来: World * viewProj）
            instancingPtr_[i].color = p.color;
            ++i;
            if (i >= instanceCount_) break;
        }
    }
}