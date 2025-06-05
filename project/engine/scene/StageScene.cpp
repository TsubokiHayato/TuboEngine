#include "StageScene.h"
#include"CollisionManager.h"

#include "CollisionManager.h"
#include "StageScene.h"

void StageScene::Initialize(Object3dCommon* object3dCommon, SpriteCommon* spriteCommon, ParticleCommon* particleCommon, WinApp* winApp, DirectXCommon* dxCommon) {
	this->object3dCommon = object3dCommon;
	this->spriteCommon = spriteCommon;
	this->winApp = winApp;
	this->dxCommon = dxCommon;
	// カメラ
	camera = std::make_unique<Camera>();
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

	// Enemyリスト
	enemies.clear();
	const int enemyCount = 5;
	for (int i = 0; i < enemyCount; ++i) {
		auto enemy = std::make_unique<Enemy>();
		enemy->Initialize(object3dCommon);
		enemy->SetCamera(camera.get());
		// 位置をずらして配置
		enemy->SetPosition(Vector3(float(i * 2), 0.0f, 5.0f));
		
		enemies.push_back(std::move(enemy));
	}
}

void StageScene::Update() {
	camera->SetTranslate(cameraPosition);
	camera->setRotation(cameraRotation);
	camera->setScale(cameraScale);
	camera->Update();

	player->SetCamera(camera.get());
	player->Update();

	// enemiesの更新
	for (auto& enemy : enemies) {
		enemy->SetCamera(camera.get());
		enemy->Update();
	}

	// 毎フレーム当たり判定
	CollisionManager::CheckPlayerEnemiesCollision(*player, enemies, 1.0f, 1.0f);
}


void StageScene::Finalize()
{
	
}

void StageScene::Object3DDraw() {
	player->Draw();
	for (auto& enemy : enemies) {
		enemy->Draw();
	}
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
