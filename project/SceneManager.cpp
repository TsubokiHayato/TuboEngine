#include "SceneManager.h"
#include"ImGuiManager.h"
#include"DebugScene.h"
#include"TitleScene.h"
#include"StageScene.h"
#include"ClearScene.h"
void SceneManager::Initialize(Object3dCommon* object3dCommon, SpriteCommon* spriteCommon, WinApp* winApp, DirectXCommon* dxCommon)
{
	this->object3dCommon = object3dCommon;
	this->spriteCommon = spriteCommon;
	this->winApp = winApp;
	this->dxCommon = dxCommon;

	currentScene = std::make_unique<DebugScene>();
	currentScene->Initialize(this->object3dCommon,this->spriteCommon,this->winApp,this->dxCommon);

	currentSceneNo = 0;
	prevSceneNo = -1;
}

void SceneManager::Update()
{
	prevSceneNo = currentSceneNo;
	currentSceneNo = currentScene->GetSceneNo();

	if (prevSceneNo != currentSceneNo) {

		if (currentScene != nullptr) {
			currentScene->Finalize();
		}

		if (currentSceneNo == DEBUG) {
			currentScene = std::make_unique<DebugScene>();
			currentScene->Initialize(object3dCommon, spriteCommon, winApp, dxCommon);
		}
		else if (currentSceneNo == TITLE) {
			currentScene = std::make_unique<TitleScene>();
			currentScene->Initialize(object3dCommon, spriteCommon, winApp, dxCommon);
		}
		else if (currentSceneNo == STAGE) {
			currentScene = std::make_unique<StageScene>();
			currentScene->Initialize(object3dCommon, spriteCommon, winApp, dxCommon);
		}
		else if (currentSceneNo == CLEAR) {
			currentScene = std::make_unique<ClearScene>();
			currentScene->Initialize(object3dCommon, spriteCommon, winApp, dxCommon);
		}
		currentScene->Initialize(object3dCommon, spriteCommon, winApp, dxCommon);
	}

	if (currentScene) {
		currentScene->Update();
	}
}

void SceneManager::Finalize()
{
	if (currentScene) {
		currentScene->Finalize();
	}

}

void SceneManager::Object3DDraw()
{
	if (currentScene) {
		currentScene->Object3DDraw();
	}
}

void SceneManager::SpriteDraw()
{
	if (currentScene) {
		currentScene->SpriteDraw();
	}
}

void SceneManager::ImGuiDraw()
{
	if (currentScene) {
		currentScene->ImGuiDraw();
	}

	ImGui::Begin("Scene");
	if (ImGui::Button("Debug")) {
		currentScene->SetSceneNo(DEBUG);
	}
	if (ImGui::Button("Title")) {
		currentScene->SetSceneNo(TITLE);
	}
	if (ImGui::Button("Stage")) {
		currentScene->SetSceneNo(STAGE);
	}
	if (ImGui::Button("Clear")) {
		currentScene->SetSceneNo(CLEAR);
	}
	ImGui::End();

}
