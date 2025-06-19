#pragma once
#include "PostEffectBase.h"
#include"DissolvePSO.h"

struct DissolveParams
{
	float dissolveThreshold; // 0.0～1.0で制御
};

class DissolveEffect : public PostEffectBase
{
public:
	DissolveEffect();
	~DissolveEffect();
	void Initialize(DirectXCommon* dxCommon) override;
	void Update() override;
	void DrawImGui() override;
	void Draw(ID3D12GraphicsCommandList* commandList) override;
	// ImGui等でパラメータを外部から変更したい場合
	DissolveParams* GetParams() { return params_; }

private:
	std::unique_ptr<DissolvePSO> pso_;
	Microsoft::WRL::ComPtr<ID3D12Resource> cbResource_;
	DissolveParams* params_ = nullptr;
};

