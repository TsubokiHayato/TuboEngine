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
	player_ = std::make_unique<Player>();
	player_->Initialize(object3dCommon);

	// 追従カメラの生成・初期化
	followCamera = std::make_unique<FollowTopDownCamera>();
	followCamera->Initialize(player_.get(), Vector3(0.0f, 10.0f, 0.0f), 0.2f);

	// プレイヤーにカメラをセット
	player_->SetCamera(followCamera->GetCamera());

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

	// 衝突マネージャの生成
	collisionManager_ = std::make_unique<CollisionManager>();
	collisionManager_->Initialize();

}

void StageScene::Update() {
	// 追従カメラの更新
	if (followCamera) {
		followCamera->Update();
	}

	player_->SetCamera(followCamera->GetCamera());
	player_->Update();

	for (auto& enemy : enemies) {
		enemy->SetCamera(followCamera->GetCamera());
		enemy->Update();
	}

	collisionManager_->Update();
	CheckAllCollisions();
}



void StageScene::Finalize()
{
	
}

void StageScene::Object3DDraw() {
	player_->Draw();
	for (auto& enemy : enemies) {
		enemy->Draw();
	}
	collisionManager_->Draw();
}
void StageScene::SpriteDraw()
{
}

void StageScene::ImGuiDraw()
{
	// CameraのImGui
	followCamera->DrawImGui();

	// PlayerのImGui
	player_->DrawImGui();


}

void StageScene::ParticleDraw()
{}

void StageScene::CheckAllCollisions() {
	/// 衝突マネージャのリセット ///
	collisionManager_->Reset();

	/// コライダーをリストに登録 ///
	collisionManager_->AddCollider(player_.get());

	// 複数の敵を登録 ///
	for (const auto& enemy : enemies) {
		collisionManager_->AddCollider(enemy.get());
	}





	// プレイヤーが死亡したらコライダーを削除または敵が死亡したらコライダーを削除 ///
	//if (player_->GetHP() <= 0 || enemy_->GetHP() <= 0) {
	//	collisionManager_->RemoveCollider(player_.get());
	//	collisionManager_->RemoveCollider(enemy_.get());

	//}

	// 衝突判定と応答
	collisionManager_->CheckAllCollisions();


}
