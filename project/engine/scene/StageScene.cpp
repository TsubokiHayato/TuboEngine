#include "StageScene.h"

void StageScene::Initialize(Object3dCommon* object3dCommon, SpriteCommon* spriteCommon, ParticleCommon* particleCommon, WinApp* winApp, DirectXCommon* dxCommon)
{
	this->object3dCommon = object3dCommon;
	this->spriteCommon = spriteCommon;
	this->winApp = winApp;
	this->dxCommon = dxCommon;
	// カメラ
	camera = std::make_unique<Camera>();
	// カメラの初期位置、回転、スケール
	cameraPosition = {0.0f, 0.0f, -5.0f};
	cameraRotation = {0.0f, 0.0f, 0.0f};
	cameraScale = {1.0f, 1.0f, 1.0f};
	
	camera->SetTranslate(cameraPosition);
	camera->setRotation(cameraRotation);
	camera->setScale(cameraScale);



	// プレイヤー
	player = std::make_unique<Player>();
	player->Initialize(object3dCommon);
	player->SetCamera(camera.get());
}

void StageScene::Update()
{
	// カメラの更新
	camera->SetTranslate(cameraPosition);
	camera->setRotation(cameraRotation);
	camera->setScale(cameraScale);
	camera->Update();
	
	player->SetCamera(camera.get());
	player->Update();
	
}

void StageScene::Finalize()
{
	
}

void StageScene::Object3DDraw()
{
	// プレイヤーの3Dオブジェクト描画
	player->Draw();
}

void StageScene::SpriteDraw()
{
}

void StageScene::ImGuiDraw()
{
	// CameraのImGui
	ImGui::Begin("camera");
	ImGui::DragFloat3("Position", &cameraPosition.x, 0.1f);
	ImGui::DragFloat3("Rotation", &cameraRotation.x, 0.1f);
	ImGui::DragFloat3("Scale", &cameraScale.x, 0.1f);
	ImGui::End();

	// PlayerのImGui
	player->DrawImgui();


}

void StageScene::ParticleDraw()
{
}
