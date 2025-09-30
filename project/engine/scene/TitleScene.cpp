#include "TitleScene.h"
#include "ImGuiManager.h"
#include "Input.h"
#include "TextureManager.h"
#include "numbers"
#include"LineManager.h"
void TitleScene::Initialize() {

	// カメラ
	camera = std::make_unique<Camera>();
	camera->SetTranslate({0.0f, 4.0f, -12.0f});
	camera->setRotation({2.4f, 0.0f, 0.0f});
	camera->setScale({1.0f, 1.0f, 1.0f});

	// Blenderシーンローダーの生成とロード
	blenderSceneLoader_ = std::make_unique<BlenderSceneLoader>();
	blenderSceneLoader_->Load("title"); // 例: resources/levels/stage/title.json
	blenderSceneLoader_->CreateObject();

	/*Transform cameraTransform = blenderSceneLoader_->GetCameraTransform();
	cameraPosition = cameraTransform.translate;
	cameraRotation = cameraTransform.rotate;
	cameraScale = cameraTransform.scale;*/

}

void TitleScene::Update() {
	// カメラ
	

	camera->SetTranslate(cameraPosition);
	camera->setRotation(cameraRotation);
	camera->setScale(cameraScale);
	camera->Update();

	if (blenderSceneLoader_) {
		blenderSceneLoader_->SetCamera(camera.get());
		blenderSceneLoader_->Update();
	}
}

void TitleScene::Finalize() { blenderSceneLoader_.reset(); }

void TitleScene::Object3DDraw() {

	// Blenderシーンの描画
	if (blenderSceneLoader_) {
		blenderSceneLoader_->Draw();
	}

	LineManager::GetInstance()->DrawGrid(10000.0f, 1000, {0.0f, 0.0f, 0.0f, 1.0f});
}

void TitleScene::SpriteDraw() {}

void TitleScene::ImGuiDraw() {
	// カメラ
	ImGui::Begin("camera");
	ImGui::DragFloat3("Position", &cameraPosition.x, 0.01f);
	ImGui::DragFloat3("Rotation", &cameraRotation.x, 0.01f);
	ImGui::DragFloat3("Scale", &cameraScale.x, 0.01f);
	ImGui::End();

	blenderSceneLoader_->DrawImgui();

	Input::GetInstance()->ShowInputDebugWindow();
}

void TitleScene::ParticleDraw() {}
