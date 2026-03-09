#include "StagePlayingState.h"
#include "LineManager.h"
#include "StageScene.h"
#include "Input.h"
#include "TextureManager.h"
#include "WinApp.h"
#include <cmath>

namespace {
	constexpr const char* kPauseGuideTex = "Pose.png";
}

void StagePlayingState::Enter(StageScene* scene) {
	if (!scene) return;
	if (StageScene::isDemoMode) {
		pauseGuideSprite_.reset();
		return;
	}
	TuboEngine::TextureManager::GetInstance()->LoadTexture(kPauseGuideTex);
	pauseGuideSprite_ = std::make_unique<TuboEngine::Sprite>();
	pauseGuideSprite_->Initialize(kPauseGuideTex);
	const float margin = 20.0f;
	TuboEngine::Math::Vector2 spriteSize = pauseGuideSprite_->GetSize() / 3;
	const float screenW = static_cast<float>(TuboEngine::WinApp::GetInstance()->GetClientWidth());
	pauseGuideSprite_->SetSize({spriteSize.x, spriteSize.y});
	pauseGuideSprite_->SetPosition({screenW - spriteSize.x, margin});
	pauseGuideSprite_->SetColor({1.0f, 1.0f, 1.0f, 0.9f});
	pauseGuideSprite_->Update();
}

void StagePlayingState::Update(StageScene* scene) {
	if (!scene) return;

	// ESCでポーズへ（デモモード時は無効）
	if (!StageScene::isDemoMode && TuboEngine::Input::GetInstance()->TriggerKey(DIK_ESCAPE)) {
		if (auto* mgr = scene->GetStageStateManager()) {
			mgr->ChangeState(StageType::Pause, scene);
		}
		return;
	}

	Player* player = scene->GetPlayer();
	FollowTopDownCamera* followCamera = scene->GetFollowCamera();
	StageManager* stageMgr = scene->GetStageManager();

	// プレイヤー設定
	if (player && followCamera) {
		player->SetCamera(followCamera->GetCamera());
		player->SetMovementLocked(false);

		// プレイヤー被弾時のカメラシェイク
		if (player->GetIsHit()) {
			followCamera->Shake(0.35f, 0.25f);
		}
		player->Update();
	}

	// ステージオブジェクト更新
	if (stageMgr && player && followCamera) {
		stageMgr->Update(player, followCamera);
	}

	// カメラ更新
	if (followCamera) {
		followCamera->Update();
		LineManager::GetInstance()->SetDefaultCamera(followCamera->GetCamera());
	}

	// --- ゲームクリア判定 / ゲームオーバー判定は StageManager 管理の敵リストから行う ---
	bool allEnemiesDefeated = true;
	bool hasAnyEnemy = false;
	if (stageMgr) {
		const auto& insts = stageMgr->GetStageInstances();
		for (const auto& inst : insts) {
			if (!inst.visible) continue;
			for (const auto& e : inst.enemies) {
				if (!e) continue;
				hasAnyEnemy = true;
				if (e->GetIsAlive()) {
					allEnemiesDefeated = false;
					break;
				}
			}
			if (!allEnemiesDefeated) break;
		}
	}

	if (allEnemiesDefeated && hasAnyEnemy) {
		// DEMOモード時はタイトルへ戻る (既存ロジックを簡略)
		if (StageScene::isDemoMode) {
			StageScene::isDemoMode = false;
			SceneManager::GetInstance()->ChangeScene(TITLE);
			return;
		}
		if (auto* mgr = scene->GetStageStateManager()) {
			mgr->ChangeState(StageType::StageClear, scene);
		}
		return;
	}

	// ゲームオーバー判定（プレイヤー死亡）
	if (player && !player->GetIsAlive()) {
		if (StageScene::isDemoMode) {
			StageScene::isDemoMode = false;
			SceneManager::GetInstance()->ChangeScene(TITLE);
			return;
		}
		if (auto* mgr = scene->GetStageStateManager()) {
			mgr->ChangeState(StageType::GameOver, scene);
		}
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
	if (!scene) return;
	// 3Dオブジェクトの描画は StageManager と SkyDome に委譲
	if (auto* stageMgr = scene->GetStageManager()) {
		stageMgr->Draw3D();
	}
	if (auto* sky = scene->GetSkyDome().get()) {
		sky->Draw();
	}
	Player* player = scene->GetPlayer();
	player->Draw();
}

void StagePlayingState::SpriteDraw(StageScene* scene) {
	if (!scene) return;
	StageManager* stageMgr = scene->GetStageManager();
	Player* player = scene->GetPlayer();

	// プレイヤーの照準
	if (player) {
		player->ReticleDraw();
	}

	// ラッシュエネミーのスプライト (Stage Manager 管理分)
	if (stageMgr) {
		const auto& insts = stageMgr->GetStageInstances();
		for (const auto& inst : insts) {
			for (const auto& e : inst.enemies) {
				if (auto* rush = dynamic_cast<RushEnemy*>(e.get())) {
					rush->DrawSprite();
				}
			}
		}
	}

	// ポーズガイド
	if (!StageScene::isDemoMode && pauseGuideSprite_) pauseGuideSprite_->Draw();
}

void StagePlayingState::ImGuiDraw(StageScene* scene) {
	if (StageScene::isDemoMode) return;
	if (!scene) return;

	if (scene->GetFollowCamera()) scene->GetFollowCamera()->DrawImGui();

#ifdef USE_IMGUI
	ImGui::Begin("Stage Playing");
	ImGui::Text("Playing State");
	ImGui::Checkbox("Auto Play (Playing)", &autoPlayEnabled_);
	ImGui::End();

	if (scene->GetPlayer()) {
		scene->GetPlayer()->DrawImGui();
	}
#endif

	// Enemy/Block ImGui は StageManager 側のインスタンスを対象にする
	if (auto* stageMgr = scene->GetStageManager()) {
		const auto& insts = stageMgr->GetStageInstances();
		for (const auto& inst : insts) {
			for (const auto& e : inst.enemies) {
				if (e) e->DrawImGui();
			}
			for (const auto& b : inst.blocks) {
				if (b) b->DrawImGui();
			}
		}
	}
}

void StagePlayingState::ParticleDraw(StageScene* scene) {
	if (!scene) return;
	if (auto* stageMgr = scene->GetStageManager()) {
		const auto& insts = stageMgr->GetStageInstances();
		for (const auto& inst : insts) {
			for (const auto& e : inst.enemies) {
				if (e) e->ParticleDraw();
			}
		}
	}
}
