#include "StageScene.h"

#include "CollisionManager.h"
#include "StageScene.h"
#include "application/FollowTopDownCamera.h"

void StageScene::Initialize() {
	
	// プレイヤー
	player_ = std::make_unique<Player>();
	player_->Initialize();

	// 追従カメラの生成・初期化
	followCamera = std::make_unique<FollowTopDownCamera>();
	followCamera->Initialize(player_.get(), Vector3(0.0f, 40.0f, 0.0f), 0.2f);

	// プレイヤーにカメラをセット
	player_->SetCamera(followCamera->GetCamera());

	// Enemyリスト
	enemies.clear();
	const int enemyCount = 1;
	for (int i = 0; i < enemyCount; ++i) {
		auto enemy = std::make_unique<Enemy>();
		enemy->Initialize();
		enemy->SetCamera(followCamera->GetCamera());
		enemy->SetPosition(Vector3(float(i * 2), 0.0f, 5.0f));
		enemies.push_back(std::move(enemy));
	}

	// 衝突マネージャの生成
	collisionManager_ = std::make_unique<CollisionManager>();
	collisionManager_->Initialize();

}

void StageScene::Update()
{


	player_->SetCamera(followCamera->GetCamera());
	player_->Update();

	
	for (auto& enemy : enemies) {
		enemy->SetCamera(followCamera->GetCamera());
		enemy->Update();
	}
	followCamera->Update();

	collisionManager_->Update();
	CheckAllCollisions();
}



void StageScene::Finalize()
{
}

void StageScene::Object3DDraw() {
	// 3Dオブジェクトの描画
	// プレイヤーの3Dオブジェクトを描画
	player_->Draw();

	// 敵の3Dオブジェクトを描画
	for (auto& enemy : enemies) {
		enemy->Draw();
	}
	// 当たり判定の可視化
	collisionManager_->Draw();
}
void StageScene::SpriteDraw() { player_->ReticleDraw(); }

void StageScene::ImGuiDraw()
{
	// CameraのImGui
	followCamera->DrawImGui();

	// PlayerのImGui
	player_->DrawImGui();


	//EnemyのImgui
	for (auto& enemy : enemies) {
		enemy->DrawImGui();
	}


}

void StageScene::ParticleDraw() {
	for (auto& enemy : enemies) {
		enemy->ParticleDraw();
	}
	}

void StageScene::CheckAllCollisions() {
	/// 衝突マネージャのリセット ///
	collisionManager_->Reset();

	/// コライダーをリストに登録 ///
	collisionManager_->AddCollider(player_.get());

	// 複数の敵を登録 ///
	for (const auto& enemy : enemies) {
		collisionManager_->AddCollider(enemy.get());
	}

	  // プレイヤーの弾
	for (const auto& bullet : player_->GetBullets()) {
		collisionManager_->AddCollider(bullet.get());
	}



	// プレイヤーが死亡したらコライダーを削除または敵が死亡したらコライダーを削除 ///
	//if (player_->GetHP() <= 0 || enemy_->GetHP() <= 0) {
	//	collisionManager_->RemoveCollider(player_.get());
	//	collisionManager_->RemoveCollider(enemy_.get());

	//}

	// 衝突判定と応答
	collisionManager_->CheckAllCollisions();


}
