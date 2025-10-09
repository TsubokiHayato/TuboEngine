#include "GameOverState.h"
#include "StageScene.h"

/// GameOverState ///
void GameOverState::Enter(StageScene* scene) {}

void GameOverState::Update(StageScene* scene) {

	// ゲームオーバーになったらシーンを切り替える
	scene->ChangeNextScene(TITLE);
}

void GameOverState::Exit(StageScene* scene) {}

void GameOverState::Object3DDraw(StageScene* scene) {}

void GameOverState::SpriteDraw(StageScene* scene) {}

void GameOverState::ImGuiDraw(StageScene* scene) {}

void GameOverState::ParticleDraw(StageScene* scene) {}
