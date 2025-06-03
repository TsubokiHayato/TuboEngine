#include "NoneEffectPSO.h"

void NoneEffectPSO::Initialize(DirectXCommon* dxCommon) {
	PostEffectPSOBase::Initialize(dxCommon);
	CreateGraphicPipeline();
}

void NoneEffectPSO::CreateGraphicPipeline() {

	PostEffectPSOBase::CreateGraphicPipeline(
		L"Resources/Shaders/PostEffect/CopyImage.VS.hlsl",
		L"Resources/Shaders/PostEffect/CopyImage.PS.hlsl"
	);
}

