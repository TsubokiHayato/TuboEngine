#include "SpriteCommon.h"
#include"PSO.h"
#include"NoneBlendPSO.h"
#include"NormalBlendPSO.h"
#include"AddBlendPSO.h"
#include"MultiplyBlendPSO.h"
#include"SubtractBlendPSO.h"
#include"ScreenBlendPSO.h"
void SpriteCommon::Initialize()
{
 
    /*---------------------------------------
        PSOの初期化
    ---------------------------------------*/
    pso = new PSO();
    pso->Initialize();
    // NoneBlendPSOの初期化
    noneBlendPSO = new NoneBlendPSO();
    noneBlendPSO->Initialize();
    // NormalBlendPSOの初期化
    normalBlendPSO = new NormalBlendPSO();
    normalBlendPSO->Initialize();
    // AddBlendPSOの初期化
    addBlendPSO = new AddBlendPSO();
    addBlendPSO->Initialize();
    // MultiplyBlendPSOの初期化
    multiplyBlendPSO = new MultiplyBlendPSO();
    multiplyBlendPSO->Initialize();
    // SubtractBlendPSOの初期化
    subtractBlendPSO = new SubtractBlendPSO();
    subtractBlendPSO->Initialize();
    // ScreenBlendPSOの初期化
    screenBlendPSO = new ScreenBlendPSO();
    screenBlendPSO->Initialize();
}

void SpriteCommon::DrawSettingsCommon(int blendMode)
{
    // PSOの共通描画設定
    switch (blendMode) {
    case 0:
        // NoneBlendPSO
        noneBlendPSO->DrawSettingsCommon();
        break;
    case 1:
        // NormalBlendPSO
        normalBlendPSO->DrawSettingsCommon();
        break;
    case 2:
        // AddBlendPSO
        addBlendPSO->DrawSettingsCommon();
        break;
    case 3:
        // SubtractBlendPSO
        subtractBlendPSO->DrawSettingsCommon();
        break;
    case 4:
        // MultiplyBlendPSO
        multiplyBlendPSO->DrawSettingsCommon();
        break;
    case 5:
        // ScreenBlendPSO
        screenBlendPSO->DrawSettingsCommon();
        break;
    default:
        // デフォルトはNoneBlendPSO
        noneBlendPSO->DrawSettingsCommon();
        break;
    }
}

