#include "Object3dCommon.h"
#include"PSO.h"
void Object3dCommon::Initialize(DirectXCommon* dxCommon)
{
	assert(dxCommon);
	dxCommon_ = dxCommon;
	pso = new PSO();
	pso->Initialize(dxCommon_);
	
}

void Object3dCommon::DrawSettingsCommon()
{

	pso->DrawSettingsCommon();
}
