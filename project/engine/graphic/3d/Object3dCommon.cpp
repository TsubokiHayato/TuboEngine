#include "Object3dCommon.h"
#include"PSO.h"
void Object3dCommon::Initialize(WinApp* winApp,DirectXCommon* dxCommon)
{
	// 引数がnullptrでないことを確認
	assert(dxCommon);
	assert(winApp);
	// 引数で受け取ってメンバ変数に記録する
	winApp_ = winApp;
	dxCommon_ = dxCommon;
	// PSOの初期化
	pso = new PSO();
	pso->Initialize(dxCommon_);

	
	
}

void Object3dCommon::DrawSettingsCommon()
{
	// PSOの共通描画設定
	pso->DrawSettingsCommon();
}
