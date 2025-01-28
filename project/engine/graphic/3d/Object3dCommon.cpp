#include "Object3dCommon.h"
#include"PSO.h"
#include"NoneBlendPSO.h"
#include"NormalBlendPSO.h"
#include"AddBlendPSO.h"
#include"SubtractBlendPSO.h"
#include"MultiplyBlendPSO.h"
#include"ScreenBlendPSO.h"

void Object3dCommon::Initialize(WinApp* winApp, DirectXCommon* dxCommon)
{
	// 引数がnullptrでないことを確認
	assert(dxCommon);
	assert(winApp);
	// 引数で受け取ってメンバ変数に記録する
	winApp_ = winApp;
	dxCommon_ = dxCommon;

	/*---------------------------------------
		PSOの初期化
	---------------------------------------*/
	pso = new PSO();
	pso->Initialize(dxCommon_);
	// NoneBlendPSOの初期化
	noneBlendPSO = new NoneBlendPSO();
	noneBlendPSO->Initialize(dxCommon_);
	// NormalBlendPSOの初期化
	normalBlendPSO = new NormalBlendPSO();
	normalBlendPSO->Initialize(dxCommon_);
	// AddBlendPSOの初期化
	addBlendPSO = new AddBlendPSO();
	addBlendPSO->Initialize(dxCommon_);
	// MultiplyBlendPSOの初期化
	multiplyBlendPSO = new MultiplyBlendPSO();
	multiplyBlendPSO->Initialize(dxCommon_);
	// SubtractBlendPSOの初期化
	subtractBlendPSO = new SubtractBlendPSO();
	subtractBlendPSO->Initialize(dxCommon_);
	// ScreenBlendPSOの初期化
	screenBlendPSO = new ScreenBlendPSO();
	screenBlendPSO->Initialize(dxCommon_);



}

void Object3dCommon::DrawSettingsCommon(int blendMode)
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