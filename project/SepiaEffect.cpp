#include "SepiaEffect.h"


void SepiaEffect::Initialize(DirectXCommon* dxCommon) {
    pso_ = std::make_unique<SepiaPSO>();
    pso_->Initialize(dxCommon);
}

void SepiaEffect::Draw(ID3D12GraphicsCommandList* commandList) {
    pso_->DrawSettingsCommon();
    // SRV等のバインドはマネージャ側で
}
