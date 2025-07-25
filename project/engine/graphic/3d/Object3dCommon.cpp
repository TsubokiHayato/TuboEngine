#include "Object3dCommon.h"
Object3dCommon* Object3dCommon::instance = nullptr; // シングルトンインスタンス
void Object3dCommon::Initialize() {
	// 引数がnullptrでないことを確認

	
	/*---------------------------------------
		PSOの初期化
	---------------------------------------*/
	pso = std::make_unique<PSO>();
	pso->Initialize();
	// NormalBlendPSOの初期化
	normalBlendPSO = std::make_unique<NormalBlendPSO>();
	normalBlendPSO->Initialize();
	// AddBlendPSOの初期化
	addBlendPSO = std::make_unique<AddBlendPSO>();
	addBlendPSO->Initialize();
	// MultiplyBlendPSOの初期化
	multiplyBlendPSO = std::make_unique<MultiplyBlendPSO>();
	multiplyBlendPSO->Initialize();
	// SubtractBlendPSOの初期化
	subtractBlendPSO = std::make_unique<SubtractBlendPSO>();
	subtractBlendPSO->Initialize();
	// ScreenBlendPSOの初期化
	screenBlendPSO = std::make_unique<ScreenBlendPSO>();
	screenBlendPSO->Initialize();
	



}

void Object3dCommon::Finalize() {

	delete instance;
	instance = nullptr;

}

void Object3dCommon::DrawSettingsCommon(int blendMode) {
	// PSOの共通描画設定
	switch (blendMode) {
	case 0:
		// NoneBlendPSO
		pso->DrawSettingsCommon();
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
		// デフォルトはpso
		pso->DrawSettingsCommon();
		break;
	}

}
