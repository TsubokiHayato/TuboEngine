#include "skyBoxCommon.h"
#include"SkyBoxPSO.h"
SkyBoxCommon* SkyBoxCommon::instance = nullptr; // シングルトンインスタンス
void SkyBoxCommon::Initialize() {
  
    /*---------------------------------------
        PSOの初期化
    ---------------------------------------*/
	pso = std::make_unique<SkyBoxPSO>();
    pso->Initialize();
   
}

void SkyBoxCommon::Finalize() {
	delete instance;
	instance = nullptr;
}

void SkyBoxCommon::DrawSettingsCommon() {
	pso->DrawSettingsCommon();
}

