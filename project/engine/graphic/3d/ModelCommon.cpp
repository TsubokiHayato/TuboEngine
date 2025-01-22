#include "ModelCommon.h"
#include<cassert>
void ModelCommon::Initialize(DirectXCommon* dxCommon)
{
	// DirectX共通部分がnullptrでないことを確認
	assert(dxCommon);
	// DirectX共通部分を設定
	dxCommon_ = dxCommon;
}
