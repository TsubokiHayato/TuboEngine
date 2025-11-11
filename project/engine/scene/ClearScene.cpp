#include "ClearScene.h"
#include "Animator.h"

void ClearScene::Initialize() {
	// カメラの初期化
	camera = std::make_unique<Camera>();
	cameraTransform.rotate = {0.0f, 0.0f, 0.0f};      // カメラの回転を初期化
	cameraTransform.scale = {1.0f, 1.0f, 1.0f};       // カメラのスケールを初期化
	cameraTransform.translate = {0.0f, 0.0f, -15.0f}; // カメラの位置を初期化
	camera->SetTranslate(cameraTransform.translate);
	camera->setRotation(cameraTransform.rotate);
	camera->setScale(cameraTransform.scale);

	animator = std::make_unique<Animator>();
	const std::string modelFileNamePath = "AnimatedCube.gltf"; // モデルファイル名
	animator->Initialize(modelFileNamePath);
	transform_.translate = {};
	transform_.scale = {1.0f, 1.0f, 1.0f};
	transform_.rotate = {0.0f, 0.0f, 0.0f};
	animator->SetPosition(transform_.translate);
	animator->SetScale(transform_.scale);
	animator->SetRotation(transform_.rotate);

}
void ClearScene::Update() {
	animator->SetCamera(camera.get());                           // カメラを設定
	animator->Update();                                          // オブジェクトの更新


	// カメラの更新
	camera->SetTranslate(cameraTransform.translate); // カメラの位置を設定
	camera->setRotation(cameraTransform.rotate);     // カメラの回転を設定
	camera->setScale(cameraTransform.scale);         // カメラのスケールを設定
	camera->Update();                                // カメラの更新
}

void ClearScene::Finalize() {}

void ClearScene::Object3DDraw() { animator->Draw(); }

void ClearScene::SpriteDraw() {}

void ClearScene::ImGuiDraw() {

#ifdef USE_IMGUI
	ImGui::Begin("ClearScene");
	ImGui::Text("Clear Scene");
	ImGui::End();

	ImGui::Begin("Camera");
	ImGui::DragFloat3("Camera Translate", &cameraTransform.translate.x, 0.1f);
	ImGui::DragFloat3("Camera Rotate", &cameraTransform.rotate.x, 0.01f);
	ImGui::DragFloat3("Camera Scale", &cameraTransform.scale.x, 0.01f);
	ImGui::End();

	ImGui::Begin("Animator");
	ImGui::Text("Animator Object");
	animator->DrawImGui("Animator Object");
	ImGui::End();
#endif // USE_IMGUI
}

void ClearScene::ParticleDraw() {}
