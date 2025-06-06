#include "OutlineEffect.h"

void OutlineEffect::Initialize(DirectXCommon* dxCommon) {

	pso_ = std::make_unique<OutlinePSO>();
	pso_->Initialize(dxCommon);

	// 定数バッファの作成
	cbResource_ = dxCommon->CreateBufferResource(sizeof(OutlineParams));
	cbResource_->Map(0, nullptr, reinterpret_cast<void**>(&params_));
	// 初期化
	params_->projectionInverse = {};


}

void OutlineEffect::Draw(ID3D12GraphicsCommandList* commandList){

	pso_->DrawSettingsCommon();
	// SRV等のセットはマネージャ側で
	commandList->SetGraphicsRootConstantBufferView(1, cbResource_->GetGPUVirtualAddress());

}