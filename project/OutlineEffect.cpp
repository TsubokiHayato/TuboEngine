#include "OutlineEffect.h"

void OutlineEffect::Initialize(DirectXCommon* dxCommon) {

	pso_ = std::make_unique<OutlinePSO>();
	pso_->Initialize(dxCommon);

}

void OutlineEffect::Draw(ID3D12GraphicsCommandList* commandList){

	pso_->DrawSettingsCommon();
	// SRV等のセットはマネージャ側で
}