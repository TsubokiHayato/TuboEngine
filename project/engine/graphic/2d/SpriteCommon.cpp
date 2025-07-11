#include "SpriteCommon.h"

SpriteCommon* SpriteCommon::instance = nullptr; // シングルトンインスタンス
void SpriteCommon::Initialize()
{
 
    /*---------------------------------------
          PSOの初期化
      ---------------------------------------*/
    pso = std::make_unique<PSO>();
    pso->Initialize();
    //// NoneBlendPSOの初期化
    //noneBlendPSO = std::make_unique<NoneBlendPSO>();
    //noneBlendPSO->Initialize();
    //// NormalBlendPSOの初期化
    //normalBlendPSO = std::make_unique<NormalBlendPSO>();
    //normalBlendPSO->Initialize();
    //// AddBlendPSOの初期化
    //addBlendPSO = std::make_unique<AddBlendPSO>();
    //addBlendPSO->Initialize();
    //// MultiplyBlendPSOの初期化
    //multiplyBlendPSO = std::make_unique<MultiplyBlendPSO>();
    //multiplyBlendPSO->Initialize();
    //// SubtractBlendPSOの初期化
    //subtractBlendPSO = std::make_unique<SubtractBlendPSO>();
    //subtractBlendPSO->Initialize();
    //// ScreenBlendPSOの初期化
    //screenBlendPSO = std::make_unique<ScreenBlendPSO>();
    //screenBlendPSO->Initialize();


}

void SpriteCommon::Finalize() {

	delete instance;
	instance = nullptr;

}

void SpriteCommon::DrawSettingsCommon(int blendMode)
{
    //// PSOの共通描画設定
    //switch (blendMode) {
    //case 0:
    //    // NoneBlendPSO
    //    noneBlendPSO->DrawSettingsCommon();
    //    break;
    //case 1:
    //    // NormalBlendPSO
    //    normalBlendPSO->DrawSettingsCommon();
    //    break;
    //case 2:
    //    // AddBlendPSO
    //    addBlendPSO->DrawSettingsCommon();
    //    break;
    //case 3:
    //    // SubtractBlendPSO
    //    subtractBlendPSO->DrawSettingsCommon();
    //    break;
    //case 4:
    //    // MultiplyBlendPSO
    //    multiplyBlendPSO->DrawSettingsCommon();
    //    break;
    //case 5:
    //    // ScreenBlendPSO
    //    screenBlendPSO->DrawSettingsCommon();
    //    break;
    //default:
    //    // デフォルトはNoneBlendPSO
    //    noneBlendPSO->DrawSettingsCommon();
    //    break;
    //}
    pso->DrawSettingsCommon();
}

