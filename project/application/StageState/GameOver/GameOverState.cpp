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
	if (scene->GetFollowCamera()) {
		scene->GetFollowCamera()->Update();
	}

	// プレイヤーは操作停止のみ
	if (player_) {
		player_->SetMovementLocked(true);
		player_->SetModelAlpha(1.0f);
		player_->SetScale({1.0f, 1.0f, 1.0f});
		if (scene->GetFollowCamera()) {
			player_->SetCamera(scene->GetFollowCamera()->GetCamera());
		}
		player_->Update();
	}

	// 暗転スプライト準備（維持）
	TuboEngine::TextureManager::GetInstance()->LoadTexture("barrier.png");
	blackoutSprite_ = std::make_unique<TuboEngine::Sprite>();
	blackoutSprite_->Initialize("barrier.png");
	blackoutSprite_->SetSize({1280.0f, 720.0f});
	blackoutSprite_->SetColor({0.0f, 0.0f, 0.0f, 0.0f});
	blackoutSprite_->Update();

	// SkyDome初期化
	if (auto* sky = scene->GetSkyDome().get()) {
		sky->Initialize();
		if (scene->GetFollowCamera()) {
			sky->SetCamera(scene->GetFollowCamera()->GetCamera());
		}
		sky->Update();
	}
}

void GameOverState::Update(StageScene* scene) {
	if (!scene) return;

	// 各オブジェクトの更新のみ（StageManager 統合後は StageManager に一任）
	if (auto* follow = scene->GetFollowCamera()) {
		follow->Update();
	}
	if (auto* sky = scene->GetSkyDome().get()) {
		if (auto* follow = scene->GetFollowCamera()) {
			sky->SetCamera(follow->GetCamera());
		}
		sky->Update();
	}
	if (auto* stageMgr = scene->GetStageManager()) {
		// Game Over 中も最終フレームの見た目を維持するため描画更新のみ（ロジックは基本停止済み想定）
		Player* player = scene->GetPlayer();
		FollowTopDownCamera* follow = scene->GetFollowCamera();
		stageMgr->Update(player, follow);
	}
	if (player_) {
		if (auto* follow = scene->GetFollowCamera()) {
			player_->SetCamera(follow->GetCamera());
		}
		player_->Update();
	}

	// 暗転アニメーション（維持）。徐々にアルファを上げ、最大でシーン遷移
	if (blackoutSprite_) {
		float alpha = blackoutSprite_->GetColor().w;
		const float maxAlpha = 1.0f;
		alpha = std::min(maxAlpha, alpha + 0.01f);
		blackoutSprite_->SetColor({0.0f, 0.0f, 0.0f, alpha});
		blackoutSprite_->Update();
		if (alpha >= maxAlpha) {
			scene->ChangeNextScene(OVER);
		}
	}
}

void GameOverState::Exit(StageScene* scene) { (void)scene; }

void GameOverState::Object3DDraw(StageScene* scene) {
	if (!scene) return;

	// StageManager 管理のステージと SkyDome を描画
	if (auto* stageMgr = scene->GetStageManager()) {
		stageMgr->Draw3D();
	}
	if (auto* sky = scene->GetSkyDome().get()) {
		sky->Draw();
	}

	// プレイヤーのみ直接描画（他の敵やブロックは StageManager 内に存在）
	if (auto* player = scene->GetPlayer()) {
		player->Draw();
	}
}

void GameOverState::SpriteDraw(StageScene* scene) {
	(void)scene;
	// 暗転のみ描画（維持）
	if (blackoutSprite_) {
		blackoutSprite_->Draw();
	}
}

void GameOverState::ImGuiDraw(StageScene* scene) { (void)scene; }

void GameOverState::ParticleDraw(StageScene* scene) { (void)scene; }
