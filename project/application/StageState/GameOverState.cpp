#include "GameOverState.h"
#include "Animation/SceneChangeAnimation.h"
#include "Character/Player/Player.h"
#include "LineManager.h"
#include "StageScene.h"
#include "engine/graphic/3d/Object3d.h"
#include <algorithm>
#include <cmath>

/// GameOverState ///
void GameOverState::Enter(StageScene* scene) {
	player_ = scene->GetPlayer();
	elapsed_ = 0.0f;
	phase_ = Phase::None;

	// フォローカメラ初期化（演出は無し）
	scene->GetFollowCamera()->Update();

	// プレイヤーは操作停止のみ
	if (player_) {
		player_->SetIsDontMove(true);
		player_->SetModelAlpha(1.0f);
		player_->SetScale({1.0f, 1.0f, 1.0f});
		player_->SetCamera(scene->GetFollowCamera()->GetCamera());
		player_->Update();
	}

	// 暗転スプライト準備（維持）
	TextureManager::GetInstance()->LoadTexture("barrier.png");
	blackoutSprite_ = std::make_unique<Sprite>();
	blackoutSprite_->Initialize("barrier.png");
	blackoutSprite_->SetSize({1280.0f, 720.0f});
	blackoutSprite_->SetColor({0.0f, 0.0f, 0.0f, 0.0f});
	blackoutSprite_->Update();

	// SkyDome初期化
	scene->GetSkyDome()->Initialize();
	scene->GetSkyDome()->SetCamera(scene->GetFollowCamera()->GetCamera());
	scene->GetSkyDome()->Update();
}

void GameOverState::Update(StageScene* scene) {
	// 各オブジェクトの更新のみ
	if (scene->GetFollowCamera())
		scene->GetFollowCamera()->Update();
	if (player_) {
		player_->SetCamera(scene->GetFollowCamera()->GetCamera());
		player_->Update();
	}
	scene->GetSkyDome()->SetCamera(scene->GetFollowCamera()->GetCamera());
	scene->GetSkyDome()->Update();

	scene->GetTile()->SetCamera(scene->GetFollowCamera()->GetCamera());
	scene->GetTile()->Update();

	for (auto& block : scene->GetBlocks()) {
		block->SetCamera(scene->GetFollowCamera()->GetCamera());
		block->Update();
	}

	for (auto& enemy : scene->GetEnemies()) {
		enemy->SetCamera(scene->GetFollowCamera()->GetCamera());
		enemy->Update();
	}

	// 暗転アニメーション（維持）。徐々にアルファを上げ、最大でシーン遷移
	float alpha = blackoutSprite_->GetColor().w;
	const float maxAlpha = 1.0f;
	alpha = std::min(maxAlpha, alpha + 0.01f);
	blackoutSprite_->SetColor({0.0f, 0.0f, 0.0f, alpha});
	blackoutSprite_->Update();
	if (alpha >= maxAlpha) {
		scene->ChangeNextScene(OVER);
	}
}

void GameOverState::Exit(StageScene* scene) {}

void GameOverState::Object3DDraw(StageScene* scene) {
	// 各オブジェクトの描画のみ
	// 3Dオブジェクト描画
	for (auto& block : scene->GetBlocks())
		block->Draw();
	player_->Draw();
	for (auto& enemy : scene->GetEnemies())
		enemy->Draw();

	scene->GetTile()->Draw();

	scene->GetSkyDome()->Draw();
}

void GameOverState::SpriteDraw(StageScene* scene) {
	// 暗転のみ描画（維持）
	if (blackoutSprite_)
		blackoutSprite_->Draw();
}

void GameOverState::ImGuiDraw(StageScene* scene) {}

void GameOverState::ParticleDraw(StageScene* scene) {}
