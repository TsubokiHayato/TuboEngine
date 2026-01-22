#include "Object3dCommon.h"
Object3dCommon* Object3dCommon::instance = nullptr; // シングルトンインスタンス
void Object3dCommon::Initialize() {
	// 引数がnullptrでないことを確認

	
	/*---------------------------------------
		PSOの初期化
	---------------------------------------*/
	pso = std::make_unique<PSO>();
	pso->Initialize();

	blendPso_ = std::make_unique<BlendPSO>();
	blendPso_->Initialize(kBlendModeNormal);
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
		blendPso_->SetBlendMode(kBlendModeNormal);
		blendPso_->DrawSettingsCommon();
		break;
	case 2:
		blendPso_->SetBlendMode(kBlendModeAdd);
		blendPso_->DrawSettingsCommon();
		break;
	case 3:
		blendPso_->SetBlendMode(kBlendModeSubtract);
		blendPso_->DrawSettingsCommon();
		break;
	case 4:
		blendPso_->SetBlendMode(kBlendModeMultily);
		blendPso_->DrawSettingsCommon();
		break;
	case 5:
		blendPso_->SetBlendMode(kBlendModeScreen);
		blendPso_->DrawSettingsCommon();
		break;
	default:
		// デフォルトはpso
		pso->DrawSettingsCommon();
		break;
	}

}
