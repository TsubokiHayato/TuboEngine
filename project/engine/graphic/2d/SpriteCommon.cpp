#include "SpriteCommon.h"
#include"PSO.h"
void SpriteCommon::Initialize(WinApp* winApp,DirectXCommon* dxCommon)
{
	winApp_ = winApp;
	dxCommon_ = dxCommon;
	pso = new PSO();
	pso->Initialize(dxCommon);
}

void SpriteCommon::DrawSettingsCommon()
{
	pso->DrawSettingsCommon();
}

