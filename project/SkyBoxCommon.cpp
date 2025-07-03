#include "skyBoxCommon.h"
#include"SkyBoxPSO.h"
void SkyBoxCommon::Initialize(WinApp* winApp, DirectXCommon* dxCommon) {
    winApp_ = winApp;
    dxCommon_ = dxCommon;

    /*---------------------------------------
        PSOの初期化
    ---------------------------------------*/
    pso = new SkyBoxPSO();
    pso->Initialize(dxCommon_);
   
}

void SkyBoxCommon::DrawSettingsCommon() {
	pso->DrawSettingsCommon();
}

