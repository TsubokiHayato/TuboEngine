#include "ParticleCommon.h"

ParticleCommon* ParticleCommon::instance = nullptr; // シングルトンインスタンス
void ParticleCommon::Initialize()
{
	
	// PSOの初期化
	pso = std::make_unique<ParticlePSO>();
	pso->Initialize();

	

}
void ParticleCommon::Finalize() {
	
	delete instance;
	instance = nullptr;
	
}
void ParticleCommon::DrawSettingsCommon()
{
	// PSOの共通描画設定
	pso->DrawSettingsCommon();

}
