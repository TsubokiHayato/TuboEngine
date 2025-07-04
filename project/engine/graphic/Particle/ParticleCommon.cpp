#include "ParticleCommon.h"
#include"ParticlePSO.h"
void ParticleCommon::Initialize(SrvManager* srvManager)
{
	//引数がnullptrでないかチェック
	
	
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
