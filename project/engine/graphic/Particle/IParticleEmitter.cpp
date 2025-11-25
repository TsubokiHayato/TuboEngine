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
	vertices_.clear();
	BuildGeometry(vertices_);

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
	uint32_t remain = (preset_.maxInstances > instanceCount_)
		? (preset_.maxInstances - instanceCount_)
		: 0;
	if (remain == 0) {
		// 容量到達。Manager側でステータスメッセージを出すのでここでは何もしない
		return;
	}

	uint32_t emitCount = std::min(remain, count);
	for (uint32_t i = 0; i < emitCount; ++i) {
		PushParticle(GenerateParticle());
	}
}

void IParticleEmitter::Update(float dt, const Camera* camera) {
	if (dt <= 0.0f) return;

	Matrix4x4 viewProj = MakeIdentity4x4();
	Matrix4x4 billboard = MakeIdentity4x4();

	if (camera) {
		viewProj = camera->GetViewProjectionMatrix();
		Matrix4x4 camWorld = camera->GetWorldMatrix();
		Matrix4x4 backToFront = MakeRotateYMatrix(std::numbers::pi_v<float>);
		billboard = Multiply(backToFront, camWorld);
		billboard.m[3][0] = 0.0f;
		billboard.m[3][1] = 0.0f;
		billboard.m[3][2] = 0.0f;
	}

	UpdateParticles(dt, viewProj, billboard);
}

void IParticleEmitter::Draw(ID3D12GraphicsCommandList* cmd) {
	if (!cmd || instanceCount_ == 0 || vertices_.empty()) return;
	if (!vb_ || !material_ || instancingSrvIndex_ < 0 || textureSrvIndex_ < 0) return;

	D3D12_VERTEX_BUFFER_VIEW vbv{};
	vbv.BufferLocation = vb_->GetGPUVirtualAddress();
	vbv.SizeInBytes = static_cast<UINT>(sizeof(VertexData) * vertices_.size());
	vbv.StrideInBytes = sizeof(VertexData);
	cmd->IASetVertexBuffers(0, 1, &vbv);

	cmd->SetGraphicsRootConstantBufferView(0, material_->GetGPUVirtualAddress());
	cmd->SetGraphicsRootDescriptorTable(1, SrvManager::GetInstance()->GetGPUDescriptorHandle(instancingSrvIndex_));
	cmd->SetGraphicsRootDescriptorTable(2, SrvManager::GetInstance()->GetGPUDescriptorHandle(textureSrvIndex_));

	cmd->DrawInstanced(static_cast<UINT>(vertices_.size()), instanceCount_, 0, 0);
}

void IParticleEmitter::DrawImGui() {
#ifdef USE_IMGUI
	// 旧個別 UI（必要なら呼び出し可能）
	if (ImGui::TreeNode(preset_.name.c_str())) {
		ImGui::Text("Instances: %u / %u", instanceCount_, preset_.maxInstances);
		ImGui::DragInt("MaxInstances", reinterpret_cast<int*>(&preset_.maxInstances), 1, 1, 50000);
		if (ImGui::Button("ApplyResize")) {
			ReallocateInstanceBufferIfNeeded();
		}
		ImGui::TreePop();
	}
#endif
}

void IParticleEmitter::EnsureBuffers() {
	auto* dx = DirectXCommon::GetInstance();

	if (!vb_ && !vertices_.empty()) {
		vb_ = dx->CreateBufferResource(sizeof(VertexData) * vertices_.size());
		void* mapped = nullptr;
		vb_->Map(0, nullptr, &mapped);
		std::memcpy(mapped, vertices_.data(), sizeof(VertexData) * vertices_.size());
		vb_->Unmap(0, nullptr);
	}

	if (!instancing_) {
		instancing_ = dx->CreateBufferResource(sizeof(ParticleForGPU) * preset_.maxInstances);
		allocatedInstances_ = preset_.maxInstances;
		instancing_->Map(0, nullptr, reinterpret_cast<void**>(&instancingPtr_));
		for (uint32_t i = 0; i < preset_.maxInstances; ++i) {
			instancingPtr_[i].WVP = MakeIdentity4x4();
			instancingPtr_[i].World = MakeIdentity4x4();
			instancingPtr_[i].color = {1,1,1,1};
		}
		instancingSrvIndex_ = SrvManager::GetInstance()->Allocate() + 1;
		SrvManager::GetInstance()->CreateSRVForStructuredBuffer(
			instancingSrvIndex_, instancing_.Get(), preset_.maxInstances, sizeof(ParticleForGPU));
	}

	if (!material_) {
		material_ = dx->CreateBufferResource(sizeof(Material));
		material_->Map(0, nullptr, reinterpret_cast<void**>(&materialPtr_));
		materialPtr_->color = {1,1,1,1};
		materialPtr_->enableLighting = false;
		materialPtr_->uvTransform = MakeIdentity4x4();
	}
}

void IParticleEmitter::ReallocateInstanceBufferIfNeeded() {
	if (preset_.maxInstances == allocatedInstances_) return;
	auto* dx = DirectXCommon::GetInstance();
	// 古いバッファ解放（ComPtrで自動）。再確保
	instancing_.Reset();
	instancing_ = dx->CreateBufferResource(sizeof(ParticleForGPU) * preset_.maxInstances);
	allocatedInstances_ = preset_.maxInstances;
	instancing_->Map(0, nullptr, reinterpret_cast<void**>(&instancingPtr_));
	for (uint32_t i = 0; i < preset_.maxInstances; ++i) {
		instancingPtr_[i].WVP = MakeIdentity4x4();
		instancingPtr_[i].World = MakeIdentity4x4();
		instancingPtr_[i].color = {1,1,1,1};
	}
	// SRV再確保
	instancingSrvIndex_ = SrvManager::GetInstance()->Allocate() + 1;
	SrvManager::GetInstance()->CreateSRVForStructuredBuffer(
		instancingSrvIndex_, instancing_.Get(), preset_.maxInstances, sizeof(ParticleForGPU));

	// 粒子数が上限を超えていたら切り詰め
	while (particles_.size() > preset_.maxInstances) {
		particles_.pop_back();
	}
	instanceCount_ = static_cast<uint32_t>(particles_.size());
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
		it->transform.scale = {
			std::lerp(preset_.scaleStart.x, preset_.scaleEnd.x, t),
			std::lerp(preset_.scaleStart.y, preset_.scaleEnd.y, t),
			std::lerp(preset_.scaleStart.z, preset_.scaleEnd.z, t)
		};
		it->color = {
			std::lerp(preset_.colorStart.x, preset_.colorEnd.x, t),
			std::lerp(preset_.colorStart.y, preset_.colorEnd.y, t),
			std::lerp(preset_.colorStart.z, preset_.colorEnd.z, t),
			std::lerp(preset_.colorStart.w, preset_.colorEnd.w, t)
		};

		it->velocity.x += preset_.gravity.x * dt;
		it->velocity.y += preset_.gravity.y * dt;
		it->velocity.z += preset_.gravity.z * dt;

		if (preset_.drag > 0.0f) {
			float d = std::clamp(1.0f - preset_.drag * dt, 0.0f, 1.0f);
			it->velocity.x *= d;
			it->velocity.y *= d;
			it->velocity.z *= d;
		}

		Vector3 delta{
			it->velocity.x * dt,
			it->velocity.y * dt,
			it->velocity.z * dt
		};
		if (preset_.simulateInWorldSpace) {
			it->transform.translate = it->transform.translate + delta;
		} else {
			it->transform.translate = (it->transform.translate + delta) + preset_.emitterTransform.translate;
		}

		++it;
	}

	instanceCount_ = static_cast<uint32_t>(particles_.size());

	if (instancingPtr_ && instanceCount_ > 0) {
		uint32_t i = 0;
		for (auto& p : particles_) {
			if (i >= preset_.maxInstances) break;
			Matrix4x4 local = MakeAffineMatrix(p.transform.scale, p.transform.rotate, p.transform.translate);
			Matrix4x4 world = preset_.billboard ? Multiply(billboard, local) : local;
			instancingPtr_[i].World = world;
			instancingPtr_[i].WVP = Multiply(world, viewProj);
			instancingPtr_[i].color = p.color;
			++i;
		}
	}
}