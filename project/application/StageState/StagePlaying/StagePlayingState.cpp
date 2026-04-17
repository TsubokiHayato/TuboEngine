#include "StagePlayingState.h"
#include "LineManager.h"
#include "StageScene.h"
#include "Input.h"
#include "TextureManager.h"
#include "WinApp.h"
#include"Character/Enemy/CircusEnemy.h"
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
		TuboEngine::LineManager::GetInstance()->SetDefaultCamera(followCamera->GetCamera());
	}

	// --- ゲームクリア判定 / チャンククリア判定 ---
	bool allEnemiesInCurrentChunkDefeated = true;
	bool hasAnyEnemyInCurrentChunk = false;
	bool allChunksCleared = false;
	if (stageMgr) {
		int mainIndex = stageMgr->GetMainChunkIndex();
		const auto& insts = stageMgr->GetStageInstances();
		if (mainIndex >= 0 && mainIndex < static_cast<int>(insts.size())) {
			const auto& cur = insts[mainIndex];
			if (cur.visible) {
				for (const auto& e : cur.enemies) {
					if (!e) continue;
					hasAnyEnemyInCurrentChunk = true;
					if (e->GetIsAlive()) {
						allEnemiesInCurrentChunkDefeated = false;
						break;
					}
				}
			}
		}
		// 全チャンクの敵が全滅しているか（GameClear 用）
		allChunksCleared = stageMgr->AreAllEnemiesDefeated();
	}

	if (allEnemiesInCurrentChunkDefeated && hasAnyEnemyInCurrentChunk) {
		// DEMOモード時はシーンチェンジ演出を挟んでタイトルへ戻る
		if (StageScene::isDemoMode) {
			// 既に遷移リクエスト中なら待つ
			if (scene->GetIsRequestSceneChange()) {
				return;
			}
			if (auto* anim = scene->GetSceneChangeAnimation()) {
				// アニメがまだ動作中なら、完了(=次のSetPhase可能)まで待つ
				if (!anim->IsFinished()) {
					return;
				}
				scene->SetPendingNextScene(TITLE);
				anim->SetPhase(SceneChangeAnimation::Phase::Appearing);
				scene->SetIsRequestSceneChange(true);
				return;
			}
			// フォールバック（アニメが無い場合のみ即遷移）
			StageScene::isDemoMode = false;
			SceneManager::GetInstance()->ChangeScene(TITLE);
			return;
		}

		// 敵全滅後、プレイヤーが kExit チップの上に乗ったら Transition へ
		// （プレイヤーが自分で出口まで歩く設計）
		if (player) {
			MapChipField* field = player->GetMapChipField();
			if (field) {
				auto playerPos = player->GetPosition();
				auto idx = field->GetMapChipIndexSetByPosition(playerPos);
				MapChipType chipAtPlayer = field->GetMapChipTypeByIndex(idx.xIndex, idx.yIndex);
				if (chipAtPlayer == MapChipType::kExit) {
					if (auto* mgr = scene->GetStageStateManager()) {
						mgr->ChangeState(StageType::Transition, scene);
					}
					return;
				}
			}
		}
		// kExit に到達していなければ Playing を続行（プレイヤーが自分で歩いて行く）
	}

	// 全チャンクの敵が全滅している場合は GameClear へ
	if (allChunksCleared) {
		if (StageScene::isDemoMode) {
			if (scene->GetIsRequestSceneChange()) {
				return;
			}
			if (auto* anim = scene->GetSceneChangeAnimation()) {
				if (!anim->IsFinished()) {
					return;
				}
				scene->SetPendingNextScene(TITLE);
				anim->SetPhase(SceneChangeAnimation::Phase::Appearing);
				scene->SetIsRequestSceneChange(true);
				return;
			}
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
			if (scene->GetIsRequestSceneChange()) {
				return;
			}
			if (auto* anim = scene->GetSceneChangeAnimation()) {
				if (!anim->IsFinished()) {
					return;
				}
				scene->SetPendingNextScene(TITLE);
				anim->SetPhase(SceneChangeAnimation::Phase::Appearing);
				scene->SetIsRequestSceneChange(true);
				return;
			}
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
	StageManager* stageMgr = scene->GetStageManager();
	// CircusEnemy の ImGui 
	if (stageMgr) {
		const auto& insts = stageMgr->GetStageInstances();
		for (const auto& inst : insts) {
			for (const auto& e : inst.enemies) {
				if (auto* circus = dynamic_cast<CircusEnemy*>(e.get())) {
					circus->DrawImGui();
				}
			}
		}
	}

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
