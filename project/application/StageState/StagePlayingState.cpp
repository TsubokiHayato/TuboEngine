#include "StagePlayingState.h"
#include "LineManager.h"
#include "StageScene.h"
#include "Input.h"
#include "TextureManager.h"
#include "WinApp.h"
#include <cmath> // 追加: sqrtf など

namespace {
	constexpr const char* kPauseGuideTex = "Pose.png";
}

// StagePlayingState
void StagePlayingState::Enter(StageScene* scene) {

	// Pause guide UI (show how to open pause)
	TextureManager::GetInstance()->LoadTexture(kPauseGuideTex);
	pauseGuideSprite_ = std::make_unique<Sprite>();
	pauseGuideSprite_->Initialize(kPauseGuideTex);

	// 右上に収まるように画面サイズから計算（anchor=右上）
	const float margin = 20.0f;
	Vector2 spriteSize = pauseGuideSprite_->GetSize()/3;
	const float screenW = static_cast<float>(WinApp::GetInstance()->GetClientWidth());
	pauseGuideSprite_->SetSize({spriteSize.x, spriteSize.y});
	pauseGuideSprite_->SetPosition({screenW - spriteSize.x, margin});
	pauseGuideSprite_->SetColor({1.0f, 1.0f, 1.0f, 0.9f});
	pauseGuideSprite_->Update();
}

void StagePlayingState::Update(StageScene* scene) {

	// ESCでポーズへ
	if (Input::GetInstance()->TriggerKey(DIK_ESCAPE)) {
		if (scene && scene->GetStageStateManager()) {
			scene->GetStageStateManager()->ChangeState(StageType::Pause, scene);
		}
		return;
	}

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

	// プレイヤー被弾時にカメラシェイク
	if (player_->GetIsHit() == true) {
		// 適度な強度と時間。必要に応じて調整可能
		followCamera->Shake(0.35f, 0.25f);
	}
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
	std::unique_ptr<Tile>& tile_ = scene->GetTile();
	// カメラ設定
	tile_->SetCamera(followCamera->GetCamera());
	// 更新
	tile_->Update();

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

	if (pauseGuideSprite_) pauseGuideSprite_->Update();
	if (scene->GetSkyDome()) scene->GetSkyDome()->Update();

}

void StagePlayingState::Exit(StageScene* scene) {
	(void)scene;
	pauseGuideSprite_.reset();
}

void StagePlayingState::Object3DDraw(StageScene* scene) {
	// 3Dオブジェクトの描画
	// スカイドーム描画
	scene->GetSkyDome()->Draw();

	// タイル描画
	std::unique_ptr<Tile>& tile_ = scene->GetTile();
	tile_->Draw();

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

	// --- 追加: プレビューステージ（Stage1+）も描画（見せるだけ） ---
	auto& stages = scene->GetStageInstances();
	for (size_t i = 1; i < stages.size(); ++i) {
		auto& st = stages[i];
		if (!st.visible) {
			continue;
		}
		if (st.tile) {
			st.tile->Draw();
		}
		for (auto& b : st.blocks) {
			b->Draw();
		}
		for (auto& e : st.enemies) {
			e->Draw();
		}
	}
}

void StagePlayingState::SpriteDraw(StageScene* scene) {
	std::vector<std::unique_ptr<Enemy>>& enemies = scene->GetEnemies();
	scene->GetPlayer()->ReticleDraw();
	for (auto& enemy : enemies) {
		if (auto* rush = dynamic_cast<RushEnemy*>(enemy.get())) {
			rush->DrawSprite();
		}
	}
	if (pauseGuideSprite_) pauseGuideSprite_->Draw();
}

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
