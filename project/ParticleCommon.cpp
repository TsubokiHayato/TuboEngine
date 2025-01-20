#include "ParticleCommon.h"
#include"ParticlePSO.h"
void ParticleCommon::Initialize(WinApp* winApp, DirectXCommon* dxCommon)
{

	assert(dxCommon);
	assert(winApp);
	winApp_ = winApp;
	dxCommon_ = dxCommon;
	pso = new ParticlePSO();
	pso->Initialize(dxCommon_);

}

void ParticleCommon::DrawSettingsCommon()
{

	pso->DrawSettingsCommon();

}
