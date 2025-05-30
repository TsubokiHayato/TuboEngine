#include "DissolveEffect.h"
#include"ImGuiManager.h"

DissolveEffect::DissolveEffect() = default;
DissolveEffect::~DissolveEffect() {
	if (cbResource_ && params_) {
		cbResource_->Unmap(0, nullptr);
		params_ = nullptr;
	}
}
void DissolveEffect::Initialize(DirectXCommon* dxCommon) {
	// PSO初期化
	pso_ = std::make_unique<DissolvePSO>();
	pso_->Initialize(dxCommon);
	// 定数バッファ作成
	cbResource_ = dxCommon->CreateBufferResource(sizeof(DissolveParams));
	cbResource_->Map(0, nullptr, reinterpret_cast<void**>(&params_));
	// デフォルト値
	params_->dissolveThreshold = 0.5f;
}
void DissolveEffect::Update() {
	

}
void DissolveEffect::DrawImGui() {
	ImGui::Begin("Dissolve Effect");
	ImGui::SliderFloat("Dissolve Threshold", &params_->dissolveThreshold, 0.0f, 1.0f);
	ImGui::End();
}
void DissolveEffect::Draw(ID3D12GraphicsCommandList* commandList) {
	// PSO・ルートシグネチャ設定
	pso_->DrawSettingsCommon();
	// SRVやCBVのバインドはマネージャ側で行う場合は不要
	// ここでCBVをバインドする場合:
	commandList->SetGraphicsRootConstantBufferView(1, cbResource_->GetGPUVirtualAddress());
}

