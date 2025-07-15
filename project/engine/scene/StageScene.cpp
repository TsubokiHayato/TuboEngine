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

	camera = std::make_unique<Camera>();
	camera->SetTranslate(cameraPosition);
	camera->setRotation(cameraRotation);
	camera->setScale(cameraScale);

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

	std::string testDDSTextureHandle = "rostock_laage_airport_4k.dds";

	skyBox_ = std::make_unique<SkyBox>();
	skyBox_->Initialize(testDDSTextureHandle); // dds
	


	// 衝突マネージャの生成
	collisionManager_ = std::make_unique<CollisionManager>();
	collisionManager_->Initialize();

}

void StageScene::Update()
{
	camera->SetTranslate(cameraPosition);
	camera->setRotation(cameraRotation);
	camera->setScale(cameraScale);
	camera->Update();
	followCamera->Update();

	player_->SetCamera(followCamera->GetCamera());
	player_->Update();

	
	for (auto& enemy : enemies) {
		enemy->SetCamera(followCamera->GetCamera());
		enemy->Update();
	}



	skyBox_->SetCamera(followCamera->GetCamera());
	skyBox_->SetPosition(followCamera->GetCamera()->GetTranslate());
	skyBox_->SetRotation(followCamera->GetCamera()->GetRotation());

	skyBox_->Update();

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

	skyBox_->Draw();
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



	// SkyBoxのImGui
	ImGui::Begin("SkyBox");
	ImGui::Text("SkyBox Settings");
	ImGui::Text("Texture: %s", skyBox_->GetTextureFilePath().c_str());
	ImGui::Text("Position: (%.2f, %.2f, %.2f)", skyBox_->GetTransform().translate.x, skyBox_->GetTransform().translate.y, skyBox_->GetTransform().translate.z);
	ImGui::Text("Rotation: (%.2f, %.2f, %.2f)", skyBox_->GetTransform().rotate.x, skyBox_->GetTransform().rotate.y, skyBox_->GetTransform().rotate.z);
	ImGui::Text("Scale: (%.2f, %.2f, %.2f)", skyBox_->GetTransform().scale.x, skyBox_->GetTransform().scale.y, skyBox_->GetTransform().scale.z);
	ImGui::End();

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
