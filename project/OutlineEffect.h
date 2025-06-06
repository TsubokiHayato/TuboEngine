#pragma once
#include"PostEffectBase.h"
#include"OutlinePSO.h"

struct OutlineParams
{
    DirectX::XMFLOAT4X4 projectionInverse;
    // 必要に応じて他のパラメータも追加
};

class OutlineEffect : public PostEffectBase
{
public:
    void Initialize(DirectXCommon* dxCommon) override;
    void Draw(ID3D12GraphicsCommandList* commandList) override;
private:
    std::unique_ptr<OutlinePSO> pso_;
	Microsoft::WRL::ComPtr<ID3D12Resource> cbResource_;
	OutlineParams* params_ = nullptr;

    Microsoft::WRL::ComPtr<ID3D12Resource> depthTextureResource ;
    D3D12_SHADER_RESOURCE_VIEW_DESC depthTextureSrvDesc = {};

};