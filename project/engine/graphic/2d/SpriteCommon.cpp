#include "SpriteCommon.h"
#include"PSO.h"
void SpriteCommon::Initialize(DirectXCommon* dxCommon)
{
	dxCommon_ = dxCommon;
	pso = new PSO();
	pso->Initialize(dxCommon);
}

void SpriteCommon::DrawSettingsCommon()
{
	pso->DrawSettingsCommon();
}

