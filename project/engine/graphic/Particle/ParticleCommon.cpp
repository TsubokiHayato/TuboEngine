#include "ParticleCommon.h"
#include"ParticlePSO.h"
void ParticleCommon::Initialize(WinApp* winApp, DirectXCommon* dxCommon,SrvManager* srvManager)
{
	//引数がnullptrでないかチェック
	assert(dxCommon);
	assert(winApp);
	// 引数で受け取ってメンバ変数に記録する
	winApp_ = winApp;
	dxCommon_ = dxCommon;
	this->srvManager_ = srvManager;

	// PSOの初期化
	pso = new ParticlePSO();
	pso->Initialize(dxCommon_);

	

}
void ParticleCommon::DrawSettingsCommon()
{
	// PSOの共通描画設定
	pso->DrawSettingsCommon();

}
