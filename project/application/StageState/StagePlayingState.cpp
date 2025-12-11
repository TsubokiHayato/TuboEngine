#include "StagePlayingState.h"
#include "LineManager.h"
#include "StageScene.h"
#include <cmath>
#include "engine/graphic/PostEffect/OffscreenRendering.h"
#include "engine/graphic/PostEffect/VignetteEffect.h"

// StagePlayingState
void StagePlayingState::Enter(StageScene* scene) {
	ruleSprite_ = std::make_unique<Sprite>();
	ruleSprite_->Initialize("rule.png");
	ruleSprite_->SetPosition({0.0f, 0.0f});
	ruleSprite_->Update();
}

void StagePlayingState::Update(StageScene* scene) {

	///------------------------------------------------
	/// 各オブジェクトの取得
	///------------------------------------------------

	Player* player_ = scene->GetPlayer();
	MapChipField* mapChipField_ = scene->GetMapChipField();
	std::vector<std::unique_ptr<Block>>& blocks_ = scene->GetBlocks();
	std::vector<std::unique_ptr<RushEnemy>>& enemies = scene->GetEnemies();
	FollowTopDownCamera* followCamera = scene->GetFollowCamera();
	OffScreenRendering* off = OffScreenRendering::GetInstance();

	///------------------------------------------------
	/// 各オブジェクトの更新
	///------------------------------------------------

	/// ブロック ///
	for (auto& block : blocks_) {
		block->SetCamera(followCamera->GetCamera());
		block->Update();
	}

	/// プレイヤー ///
	player_->SetCamera(followCamera->GetCamera());
	player_->SetIsDontMove(false);
	player_->SetMapChipField(mapChipField_);

	// HPに応じて外枠の赤を控えめに調整（見づらくならない上限）
	{
		const int maxHP = 5;
		int hp = player_->GetHP();
		float hpRatio = hp / static_cast<float>(maxHP);
		if (hpRatio < 0.0f) hpRatio = 0.0f;
		if (hpRatio > 1.0f) hpRatio = 1.0f;

		off->SetCurrentPostEffect(3); // Vignette
		if (auto* vignette = off->GetPostEffect<VignetteEffect>()) {
			float basePower = 0.8f;
			float baseScale = 14.0f;
			float maxBoostPower = 3.0f;
			float maxBoostScale = 6.0f;
			vignette->GetParams()->vignettePower = basePower + (1.0f - hpRatio) * maxBoostPower;
			vignette->GetParams()->vignetteScale = baseScale + (1.0f - hpRatio) * maxBoostScale;
			vignette->GetParams()->tintR = 1.0f;
			vignette->GetParams()->tintG = 0.25f;
			vignette->GetParams()->tintB = 0.25f;
			vignette->GetParams()->tintA = 0.12f + (1.0f - hpRatio) * 0.28f;
		}
	}

	// プレイヤー被弾時にカメラシェイク + 外枠の赤みを一瞬だけ強調（視認性維持）
	if (player_->GetIsHit()) {
		followCamera->Shake(0.35f, 0.25f);
		if (auto* vignette = off->GetPostEffect<VignetteEffect>()) {
			if (vignette->GetParams()->tintA < 0.55f) {
				vignette->GetParams()->tintA = 0.55f;
			}
			if (vignette->GetParams()->vignettePower < 4.0f) {
				vignette->GetParams()->vignettePower = 4.0f;
			}
		}
	}

	// 更新
	player_->Update();

	/// 敵 ///
	for (auto& enemy : enemies) {
		enemy->SetCamera(followCamera->GetCamera());
		enemy->SetPlayer(player_);
		enemy->SetMapChipField(mapChipField_);
		enemy->Update();
	}

	/// Tile///
	std::vector<std::unique_ptr<Tile>>& tiles_ = scene->GetTiles();
	for (auto& tile : tiles_) {
		tile->SetCamera(followCamera->GetCamera());
		tile->Update();
	}

	/// カメラ ///
	followCamera->Update();

	///------------------------------------------------
	/// ゲームクリア判定
	///------------------------------------------------

	bool allEnemiesDefeated = true;
	for (const auto& enemy : enemies) {
		if (enemy->GetIsAllive()) { allEnemiesDefeated = false; break; }
	}
	if (allEnemiesDefeated && !enemies.empty()) {
		scene->GetStageStateManager()->ChangeState(StageType::StageClear, scene);
		return;
	}

	///------------------------------------------------
	/// ゲームオーバー判定
	///------------------------------------------------

	if (!player_->GetIsAllive()) {
		scene->GetStageStateManager()->ChangeState(StageType::GameOver, scene);
		return;
	}

	ruleSprite_->Update();
}

void StagePlayingState::Exit(StageScene* scene) {}

void StagePlayingState::Object3DDraw(StageScene* scene) {
	scene->GetSkyDome()->Draw();
	std::vector<std::unique_ptr<Tile>>& tiles_ = scene->GetTiles();
	for (auto& tile : tiles_) { tile->Draw(); }
	std::vector<std::unique_ptr<Block>>& blocks_ = scene->GetBlocks();
	for (auto& block : blocks_) { block->Draw(); }
	scene->GetPlayer()->Draw();
	std::vector<std::unique_ptr<RushEnemy>>& enemies = scene->GetEnemies();
	for (auto& enemy : enemies) { enemy->Draw(); }
}

void StagePlayingState::SpriteDraw(StageScene* scene) {
	std::vector<std::unique_ptr<RushEnemy>>& enemies = scene->GetEnemies();
	scene->GetPlayer()->ReticleDraw();
	for (auto& enemy : enemies) { enemy->DrawSprite(); }
	ruleSprite_->Draw();
}

void StagePlayingState::ImGuiDraw(StageScene* scene) {
	scene->GetFollowCamera()->DrawImGui();
	scene->GetPlayer()->DrawImGui();
	std::vector<std::unique_ptr<RushEnemy>>& enemies = scene->GetEnemies();
	for (auto& enemy : enemies) { enemy->DrawImGui(); }
	std::vector<std::unique_ptr<Block>>& blocks_ = scene->GetBlocks();
	for (auto& block : blocks_) { block->DrawImGui(); }
}

void StagePlayingState::ParticleDraw(StageScene* scene) {
	std::vector<std::unique_ptr<RushEnemy>>& enemies = scene->GetEnemies();
	for (auto& enemy : enemies) { enemy->ParticleDraw(); }
}
