#include "StagePlayingState.h"
#include "StageScene.h"

// StagePlayingState
void StagePlayingState::Enter(StageScene* scene) {}

void StagePlayingState::Update(StageScene* scene) {

	///------------------------------------------------
	/// 各オブジェクトの取得
	///------------------------------------------------

	Player* player_ = scene->GetPlayer();
	MapChipField* mapChipField_ = scene->GetMapChipField();
	std::vector<std::unique_ptr<Block>>& blocks_ = scene->GetBlocks();
	std::vector<std::unique_ptr<Enemy>>& enemies = scene->GetEnemies();
	FollowTopDownCamera* followCamera = scene->GetFollowCamera();

	///------------------------------------------------
	/// 各オブジェクトの更新
	///------------------------------------------------


	/// MapChipField///
	

	/// ブロック ///
	for (auto& block : blocks_) {
		// カメラ設定
		block->SetCamera(followCamera->GetCamera());
		// 更新
		block->Update();
	}

	/// プレイヤー ///
	// カメラ設定
	player_->SetCamera(followCamera->GetCamera());
	player_->SetIsDontMove(false);
	player_->SetMapChipField(mapChipField_);
	// 更新
	player_->Update();

	/// 敵 ///
	for (auto& enemy : enemies) {
		// カメラ設定
		enemy->SetCamera(followCamera->GetCamera());
		// プレイヤー設定
		enemy->SetPlayer(player_);
		// マップチップフィールド設定
		enemy->SetMapChipField(mapChipField_);
		// 更新
		enemy->Update();
	}

	/// Tile///
	std::vector<std::unique_ptr<Tile>>& tiles_ = scene->GetTiles();
	for (auto& tile : tiles_) {
		// カメラ設定
		tile->SetCamera(followCamera->GetCamera());
		// 更新
		tile->Update();
	}

	/// カメラ ///
	// 更新
	followCamera->Update();

	///------------------------------------------------
	/// ゲームクリア判定
	///------------------------------------------------

	// 全ての敵が倒されたかチェック
	bool allEnemiesDefeated = true;
	for (const auto& enemy : enemies) {
		if (enemy->GetIsAllive()) {
			allEnemiesDefeated = false;
			break;
		}
	}
	if (allEnemiesDefeated && !enemies.empty()) {
		scene->GetStageStateManager()->ChangeState(StageType::StageClear, scene);
		return;
	}

	///------------------------------------------------
	/// ゲームオーバー判定
	///------------------------------------------------

	if (!player_->GetIsAllive()) {
		// プレイヤーが死亡したらゲームオーバーステートへ
		scene->GetStageStateManager()->ChangeState(StageType::GameOver, scene);
		return;
	}





}

void StagePlayingState::Exit(StageScene* scene) {}

void StagePlayingState::Object3DDraw(StageScene* scene) {

	// 3Dオブジェクトの描画
	// ブロック描画
	std::vector<std::unique_ptr<Block>>& blocks_ = scene->GetBlocks();
	for (auto& block : blocks_) {
		block->Draw();
	}

	// プレイヤーの3Dオブジェクトを描画
	scene->GetPlayer()->Draw();

	// 敵の3Dオブジェクトを描画
	std::vector<std::unique_ptr<Enemy>>& enemies = scene->GetEnemies();
	for (auto& enemy : enemies) {
		enemy->Draw();
	}
	// タイル描画
	std::vector<std::unique_ptr<Tile>>& tiles_ =scene->GetTiles();
	for (auto& tile : tiles_) {
		tile->Draw();
	}
	// スカイドーム描画
	scene->GetSkyDome()->Draw();
}

void StagePlayingState::SpriteDraw(StageScene* scene) { scene->GetPlayer()->ReticleDraw(); }

void StagePlayingState::ImGuiDraw(StageScene* scene) {

	scene->GetFollowCamera()->DrawImGui();
	scene->GetPlayer()->DrawImGui();

	std::vector<std::unique_ptr<Enemy>>& enemies = scene->GetEnemies();
	// EnemyのImgui
	for (auto& enemy : enemies) {
		enemy->DrawImGui();
	}

	std::vector<std::unique_ptr<Block>>& blocks_ = scene->GetBlocks();
	// ブロックのImGui
	for (auto& block : blocks_) {
		block->DrawImGui();
	}
}

void StagePlayingState::ParticleDraw(StageScene* scene) {

	std::vector<std::unique_ptr<Enemy>>& enemies = scene->GetEnemies();

	for (auto& enemy : enemies) {
		enemy->ParticleDraw();
	}
}
