#include "StageScene.h"
#include"CollisionManager.h"

#include "CollisionManager.h"
#include "StageScene.h"
#include "StageScene.h"
#include "FollowTopDownCamera.h"
// ...（他のinclude）

void StageScene::Initialize(Object3dCommon* object3dCommon, SpriteCommon* spriteCommon, ParticleCommon* particleCommon, WinApp* winApp, DirectXCommon* dxCommon) {
	this->object3dCommon = object3dCommon;
	this->spriteCommon = spriteCommon;
	this->winApp = winApp;
	this->dxCommon = dxCommon;

	// プレイヤー
	player = std::make_unique<Player>();
	player->Initialize(object3dCommon);

	// 追従カメラの生成・初期化
	followCamera = std::make_unique<FollowTopDownCamera>();
	followCamera->Initialize(player.get(), Vector3(0.0f, 10.0f, 0.0f), 0.2f);

	// プレイヤーにカメラをセット
	player->SetCamera(followCamera->GetCamera());

	// Enemyリスト
	enemies.clear();
	const int enemyCount = 5;
	for (int i = 0; i < enemyCount; ++i) {
		auto enemy = std::make_unique<Enemy>();
		enemy->Initialize(object3dCommon);
		enemy->SetCamera(followCamera->GetCamera());
		enemy->SetPosition(Vector3(float(i * 2), 0.0f, 5.0f));
		enemies.push_back(std::move(enemy));
	}
}

void StageScene::Update() {
	// 追従カメラの更新
	if (followCamera) {
		followCamera->Update();
	}

	player->SetCamera(followCamera->GetCamera());
	player->Update();

	for (auto& enemy : enemies) {
		enemy->SetCamera(followCamera->GetCamera());
		enemy->Update();
	}

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
	followCamera->DrawImGui();

	// PlayerのImGui
	player->DrawImgui();


}

void StageScene::ParticleDraw()
{
}
