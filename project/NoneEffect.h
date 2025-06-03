#pragma once
#include "PostEffectBase.h"
#include "NoneEffectPSO.h"

class NoneEffect : public PostEffectBase
{
public:
    void Initialize(DirectXCommon* dxCommon) override;
    void Draw(ID3D12GraphicsCommandList* commandList) override;
private:
    std::unique_ptr<NoneEffectPSO> pso_;
};

