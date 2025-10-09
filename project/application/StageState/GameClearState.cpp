#include "GameClearState.h"
#include "StageScene.h"


/// GameClearState ///
void GameClearState::Enter(StageScene* scene) {}

void GameClearState::Update(StageScene* scene) {

	// ゲームクリアになったらシーンを切り替える
	scene->ChangeNextScene(CLEAR);

}

void GameClearState::Exit(StageScene* scene) {}

void GameClearState::Object3DDraw(StageScene* scene) {}

void GameClearState::SpriteDraw(StageScene* scene) {}

void GameClearState::ImGuiDraw(StageScene* scene) {}

void GameClearState::ParticleDraw(StageScene* scene) {}
