#include "StageScene.h"
#include "CollisionManager.h"
#include "FollowTopDownCamera.h"
#include "LineManager.h"

void StageScene::Initialize() {

	mapChipField_ = std::make_unique<MapChipField>();
	player_ = std::make_unique<Player>();
	followCamera = std::make_unique<FollowTopDownCamera>();
	collisionManager_ = std::make_unique<CollisionManager>();
	stateManager_ = std::make_unique<StageStateManager>();

	// 衝突マネージャの生成

	collisionManager_->Initialize();

	stateManager_->Initialize(this);
}

void StageScene::Update() {

	// 現在の状態のUpdateを呼ぶだけ
	if (stateManager_) {
		stateManager_->Update(this);
	}

	for (auto& block : blocks_) {

		block->SetCamera(followCamera->GetCamera());
		block->Update();
	}

	player_->SetCamera(followCamera->GetCamera());
	player_->Update();

	for (auto& enemy : enemies) {
		enemy->SetCamera(followCamera->GetCamera());
		enemy->SetPlayer(player_.get());
		enemy->SetMapChipField(mapChipField_.get());
		enemy->Update();
	}
	followCamera->Update();

	LineManager::GetInstance()->SetDefaultCamera(followCamera->GetCamera());
	collisionManager_->Update();

	if (!player_->GetIsAllive()) {
		// プレイヤーが死亡したらシーンを切り替える
		SceneManager::GetInstance()->ChangeScene(CLEAR);
		return;
	}

	CheckAllCollisions();
}

void StageScene::Finalize() {}

void StageScene::Object3DDraw() {
	// 3Dオブジェクトの描画
	// ブロック描画
	for (auto& block : blocks_) {
		block->Draw();
	}

	// プレイヤーの3Dオブジェクトを描画
	player_->Draw();

	// 敵の3Dオブジェクトを描画
	for (auto& enemy : enemies) {
		enemy->Draw();
	}
	// 当たり判定の可視化
	collisionManager_->Draw();

	// グリッド描画（回転対応）
	LineManager::GetInstance()->DrawGrid(10000.0f, 1000, {DirectX::XM_PIDIV2, 0.0f, 0.0f});
}
void StageScene::SpriteDraw() { player_->ReticleDraw(); }

void StageScene::ImGuiDraw() {
	// CameraのImGui
	followCamera->DrawImGui();

	// PlayerのImGui
	player_->DrawImGui();

	// DrawLineのImGui
	LineManager::GetInstance()->DrawImGui();
	// EnemyのImgui
	for (auto& enemy : enemies) {
		enemy->DrawImGui();
	}

	// ブロックのImGui
	for (auto& block : blocks_) {
		block->DrawImGui();
	}

	mapChipField_->DrawImGui();
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
	// if (player_->GetHP() <= 0 || enemy_->GetHP() <= 0) {
	//	collisionManager_->RemoveCollider(player_.get());
	//	collisionManager_->RemoveCollider(enemy_.get());

	//}

	// 衝突判定と応答
	collisionManager_->CheckAllCollisions();
}
