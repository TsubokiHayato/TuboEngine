#include "StageClearState.h"
#include "StageScene.h"
// StageClearState
void StageClearState::Enter(StageScene* scene) {

	restartSprite_ = std::make_unique<Sprite>();
	restartSprite_->Initialize("restart.png");
	restartSprite_->SetPosition({640.0f, 680.0f});
	restartSprite_->SetAnchorPoint({0.5f, 0.5f});
	restartSprite_->Update();
}

void StageClearState::Update(StageScene* scene) {
	if (Input::GetInstance()->TriggerKey(DIK_SPACE)) {
		// ステージシーンをリスタートする処理
		scene->GetStageStateManager()->ChangeState(StageType::Ready, scene);
	}

	restartSprite_->Update();
}

void StageClearState::Exit(StageScene* scene) {}

void StageClearState::Object3DDraw(StageScene* scene) {}

void StageClearState::SpriteDraw(StageScene* scene) { restartSprite_->Draw(); }

void StageClearState::ImGuiDraw(StageScene* scene) {}

void StageClearState::ParticleDraw(StageScene* scene) {}
