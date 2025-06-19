#pragma once
#include "PostEffectPSOBase.h"
class RadialBlurPSO : public PostEffectPSOBase
{
public:
	void Initialize(DirectXCommon* dxCommon) override;
	void CreateGraphicPipeline();
	void CreateRootSignature() override;

};


