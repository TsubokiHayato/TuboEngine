#include "GrayScaleEffect.h"

void GrayScaleEffect::Initialize(DirectXCommon* dxCommon) {
    pso_ = std::make_unique<GrayScalePSO>();
    pso_->Initialize(dxCommon);
}

void GrayScaleEffect::Draw(ID3D12GraphicsCommandList* commandList) {
    pso_->DrawSettingsCommon();
    // SRV等のセットはマネージャ側で
}
