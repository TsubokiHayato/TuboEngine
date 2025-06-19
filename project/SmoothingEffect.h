#pragma once
#include"PostEffectBase.h"
#include"SmoothingPSO.h"
class SmoothingEffect : public PostEffectBase
{
public:
    void Initialize(DirectXCommon* dxCommon) override;
    void Draw(ID3D12GraphicsCommandList* commandList) override;
private:
    std::unique_ptr<SmoothingPSO> pso_;
};


