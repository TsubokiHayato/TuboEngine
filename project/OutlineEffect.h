#pragma once
#include"PostEffectBase.h"
#include"OutlinePSO.h"
#include"MT_Matrix.h"

struct OutlineParams
{
    Matrix4x4 projectionInverse;
    // 必要に応じて他のパラメータも追加
};

class OutlineEffect : public PostEffectBase
{
public:
    void Initialize(DirectXCommon* dxCommon) override;
    void Draw(ID3D12GraphicsCommandList* commandList) override;

public:
    

	Matrix4x4 SetProjection(const Matrix4x4& projection) {
		
		// projection行列の逆行列を計算
		params_->projectionInverse = Inverse(projection);
		// 定数バッファに更新
		cbResource_->Unmap(0, nullptr);
		return params_->projectionInverse;
	}
	
private:
    std::unique_ptr<OutlinePSO> pso_;
	Microsoft::WRL::ComPtr<ID3D12Resource> cbResource_;
	OutlineParams* params_ = nullptr;

    Microsoft::WRL::ComPtr<ID3D12Resource> depthTextureResource ;
    D3D12_SHADER_RESOURCE_VIEW_DESC depthTextureSrvDesc = {};

};