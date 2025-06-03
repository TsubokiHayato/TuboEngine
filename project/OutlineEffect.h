#pragma once
#include"PostEffectBase.h"
#include"OutlinePSO.h"
class OutlineEffect : public PostEffectBase
{
public:
    void Initialize(DirectXCommon* dxCommon) override;
    void Draw(ID3D12GraphicsCommandList* commandList) override;
private:
    std::unique_ptr<OutlinePSO> pso_;

};