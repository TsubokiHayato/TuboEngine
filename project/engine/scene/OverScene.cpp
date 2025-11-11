#include "OverScene.h"

void OverScene::Initialize() {
	// カメラの初期化
	camera = std::make_unique<Camera>();
	cameraTransform.rotate = {0.0f, 0.0f, 0.0f};      // カメラの回転を初期化
	cameraTransform.scale = {1.0f, 1.0f, 1.0f};       // カメラのスケールを初期化
	cameraTransform.translate = {0.0f, 0.0f, -15.0f}; // カメラの位置を初期化
	camera->SetTranslate(cameraTransform.translate);
	camera->setRotation(cameraTransform.rotate);
	camera->setScale(cameraTransform.scale);

}
void OverScene::Update() {
	
	// カメラの更新
	camera->SetTranslate(cameraTransform.translate); // カメラの位置を設定
	camera->setRotation(cameraTransform.rotate);     // カメラの回転を設定
	camera->setScale(cameraTransform.scale);         // カメラのスケールを設定
	camera->Update();                                // カメラの更新
}

void OverScene::Finalize() {}

void OverScene::Object3DDraw() { }

void OverScene::SpriteDraw() {}

void OverScene::ImGuiDraw() {
	ImGui::Begin("OVERScene");
	ImGui::Text("OVERScene");
	ImGui::End();

	ImGui::Begin("Camera");
	ImGui::DragFloat3("Camera Translate", &cameraTransform.translate.x, 0.1f);
	ImGui::DragFloat3("Camera Rotate", &cameraTransform.rotate.x, 0.01f);
	ImGui::DragFloat3("Camera Scale", &cameraTransform.scale.x, 0.01f);
	ImGui::End();

	
}

void OverScene::ParticleDraw() {}
