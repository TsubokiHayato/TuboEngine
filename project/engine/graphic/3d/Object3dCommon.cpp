#include "Object3dCommon.h"
#include"PSO.h"
void Object3dCommon::Initialize(WinApp* winApp,DirectXCommon* dxCommon)
{
	assert(dxCommon);
	assert(winApp);
	winApp_ = winApp;
	dxCommon_ = dxCommon;
	pso = new PSO();
	pso->Initialize(dxCommon_);
	
}

void Object3dCommon::DrawSettingsCommon()
{

	pso->DrawSettingsCommon();
}
