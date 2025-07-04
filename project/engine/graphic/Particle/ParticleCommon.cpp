#include "ParticleCommon.h"
#include"ParticlePSO.h"
void ParticleCommon::Initialize()
{
	
	// PSOの初期化
	pso = new ParticlePSO();
	pso->Initialize();

	

}
void ParticleCommon::DrawSettingsCommon()
{
	// PSOの共通描画設定
	pso->DrawSettingsCommon();

}
