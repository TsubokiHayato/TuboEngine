#include "OutlinePSO.h"

void OutlinePSO::Initialize(DirectXCommon* dxCommon) {

	PostEffectPSOBase::Initialize(dxCommon);
	CreateGraphicPipeline();

}

void OutlinePSO::CreateGraphicPipeline() {
	PostEffectPSOBase::CreateGraphicPipeline(
		L"Resources/Shaders/PostEffect/CopyImage.VS.hlsl",
		L"Resources/Shaders/PostEffect/Outline.PS.hlsl"
	);
}