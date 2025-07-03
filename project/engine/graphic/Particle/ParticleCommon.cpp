#include "ParticleCommon.h"
#include"ParticlePSO.h"
void ParticleCommon::Initialize(WinApp* winApp,SrvManager* srvManager)
{
	//引数がnullptrでないかチェック
	
	assert(winApp);
	// 引数で受け取ってメンバ変数に記録する
	winApp_ = winApp;

	this->srvManager_ = srvManager;

	// PSOの初期化
	pso = new ParticlePSO();
	pso->Initialize();

	

}
void ParticleCommon::DrawSettingsCommon()
{
	// PSOの共通描画設定
	pso->DrawSettingsCommon();

}
