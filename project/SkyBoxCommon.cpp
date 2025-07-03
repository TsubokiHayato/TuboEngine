#include "skyBoxCommon.h"
#include"SkyBoxPSO.h"
void SkyBoxCommon::Initialize(WinApp* winApp) {
    winApp_ = winApp;
   

    /*---------------------------------------
        PSOの初期化
    ---------------------------------------*/
    pso = new SkyBoxPSO();
    pso->Initialize();
   
}

void SkyBoxCommon::DrawSettingsCommon() {
	pso->DrawSettingsCommon();
}

