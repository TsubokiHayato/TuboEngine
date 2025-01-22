#include "ParticleCommon.h"
#include"ParticlePSO.h"
void ParticleCommon::Initialize(WinApp* winApp, DirectXCommon* dxCommon,SrvManager* srvManager)
{

	assert(dxCommon);
	assert(winApp);
	winApp_ = winApp;
	dxCommon_ = dxCommon;
	this->srvManager_ = srvManager;
	pso = new ParticlePSO();
	pso->Initialize(dxCommon_);

	

}
void ParticleCommon::DrawSettingsCommon()
{

	pso->DrawSettingsCommon();

}
