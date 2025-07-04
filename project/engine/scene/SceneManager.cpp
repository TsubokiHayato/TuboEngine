#include "SceneManager.h"
#include"ImGuiManager.h"
#include"DebugScene.h"
#include"TitleScene.h"
#include"StageScene.h"
#include"ClearScene.h"
void SceneManager::Initialize() {
	
	//初期シーンを設定
	currentScene = std::make_unique<DebugScene>();
	currentScene->Initialize();

	//シーン番号を設定
	currentSceneNo = 0;
	//前のシーン番号を設定
	prevSceneNo = -1;
}

void SceneManager::Update() {
	//現在のシーンがnullptrでない場合
	if (currentScene == nullptr) {
		return;
	}
	//前のシーン番号を設定
	prevSceneNo = currentSceneNo;
	//現在のシーン番号を取得
	currentSceneNo = currentScene->GetSceneNo();

	//前のシーン番号と現在のシーン番号が異なる場合
	if (prevSceneNo != currentSceneNo) {
		//現在のシーンがnullptrでない場合
		if (currentScene != nullptr) {
			//終了処理
			currentScene->Finalize();
		}

		//シーン番号によってシーンを設定
		if (currentSceneNo == DEBUG) {
			//デバッグシーンを設定
			currentScene = std::make_unique<DebugScene>();
			currentScene->Initialize();
		} else if (currentSceneNo == TITLE) {
			//タイトルシーンを設定
			currentScene = std::make_unique<TitleScene>();
			currentScene->Initialize();
		} else if (currentSceneNo == STAGE) {
			//ステージシーンを設定
			currentScene = std::make_unique<StageScene>();
			currentScene->Initialize();
		} else if (currentSceneNo == CLEAR) {
			//クリアシーンを設定
			currentScene = std::make_unique<ClearScene>();
			currentScene->Initialize();
		}
		//初期化
		currentScene->Initialize();
	}
	//現在のシーンがnullptrでない場合
	if (currentScene) {
		//更新処理
		currentScene->Update();
	}
}

void SceneManager::Finalize() {
	//現在のシーンがnullptrでない場合
	if (currentScene) {
		//終了処理
		currentScene->Finalize();
	}

}

void SceneManager::Object3DDraw() {
	//現在のシーンがnullptrでない場合
	if (currentScene) {
		//3Dオブジェクト描画
		currentScene->Object3DDraw();
	}
}

void SceneManager::SpriteDraw() {
	//現在のシーンがnullptrでない場合
	if (currentScene) {
		//スプライト描画
		currentScene->SpriteDraw();
	}
}

void SceneManager::ImGuiDraw() {
	//現在のシーンがnullptrでない場合
	if (currentScene) {
		//ImGui描画
		currentScene->ImGuiDraw();
	}

	//シーン選択ウィンドウ
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

void SceneManager::ParticleDraw() {
	//現在のシーンがnullptrでない場合
	if (currentScene) {
		//パーティクル描画
		currentScene->ParticleDraw();
	}
}
