#pragma once
#include "PostEffectBase.h"
#include "SepiaPSO.h"

class SepiaEffect : public PostEffectBase
{
public:
    void Initialize(DirectXCommon* dxCommon) override;
    void Draw(ID3D12GraphicsCommandList* commandList) override;
private:
    std::unique_ptr<SepiaPSO> pso_;
};
