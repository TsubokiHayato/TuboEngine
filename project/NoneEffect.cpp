#include "NoneEffect.h"

void NoneEffect::Initialize(DirectXCommon* dxCommon) {
    pso_ = std::make_unique<NoneEffectPSO>();
    pso_->Initialize(dxCommon);
}

void NoneEffect::Draw(ID3D12GraphicsCommandList* commandList) {

	pso_->DrawSettingsCommon();
	// SRV等のセットはマネージャ側で
}
