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
		// DEMOモード時はタイトルへ戻る (既存ロジックを簡略)
		if (StageScene::isDemoMode) {
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
		if (auto* mgr = scene->GetStageStateManager()) {
			mgr->ChangeState(StageType::GameClear, scene);
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

#ifdef USE_IMGUI
	// --- デバッグ: 入口(青)・出口(黄)にマーカーを表示 ---
	if (player) {
		MapChipField* field = player->GetMapChipField();
		if (field) {
			auto entrances = field->GetChipPositions(MapChipType::kEntrance);
			auto exits = field->GetChipPositions(MapChipType::kExit);
			const float markerHeight = 3.0f;  // マーカーの高さ（縦線の長さ）
			const float sphereR = 0.8f;

			// 入口: 青い縦線 + 球
			for (const auto& pos : entrances) {
				TuboEngine::Math::Vector3 top = { pos.x, pos.y, pos.z + markerHeight };
				TuboEngine::LineManager::GetInstance()->DrawLine(pos, top, {0.2f, 0.5f, 1.0f, 1.0f}); // 青
				TuboEngine::LineManager::GetInstance()->DrawSphere(top, sphereR, {0.2f, 0.5f, 1.0f, 1.0f});
			}

			// 出口: 黄色い縦線 + 球
			for (const auto& pos : exits) {
				TuboEngine::Math::Vector3 top = { pos.x, pos.y, pos.z + markerHeight };
				TuboEngine::LineManager::GetInstance()->DrawLine(pos, top, {1.0f, 0.9f, 0.2f, 1.0f}); // 黄
				TuboEngine::LineManager::GetInstance()->DrawSphere(top, sphereR, {1.0f, 0.9f, 0.2f, 1.0f});
			}

			// 入口→出口への案内ライン（緑の点線風）
			for (const auto& ent : entrances) {
				for (const auto& ext : exits) {
					TuboEngine::Math::Vector3 entTop = { ent.x, ent.y, ent.z + markerHeight };
					TuboEngine::Math::Vector3 extTop = { ext.x, ext.y, ext.z + markerHeight };
					TuboEngine::LineManager::GetInstance()->DrawLine(entTop, extTop, {0.2f, 1.0f, 0.3f, 0.6f}); // 緑
				}
			}
		}
	}
#endif // USE_IMGUI
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

	// --- 入口・出口の目印を球体のみで描画 ---
	if (player) {
		MapChipField* field = player->GetMapChipField();
		if (field) {
			auto entrances = field->GetChipPositions(MapChipType::kEntrance);
			auto exits = field->GetChipPositions(MapChipType::kExit);
			const float markerHeight = 3.0f;  // 球体の高さ（地面から浮かせる）
			const float sphereR = 0.8f;

			// 入口: 青い球
			for (const auto& pos : entrances) {
				TuboEngine::Math::Vector3 top = { pos.x, pos.y, pos.z + markerHeight };
				TuboEngine::LineManager::GetInstance()->DrawSphere(top, sphereR, {0.2f, 0.5f, 1.0f, 1.0f});
			}

			// 出口: 黄色い球
			for (const auto& pos : exits) {
				TuboEngine::Math::Vector3 top = { pos.x, pos.y, pos.z + markerHeight };
				TuboEngine::LineManager::GetInstance()->DrawSphere(top, sphereR, {1.0f, 0.9f, 0.2f, 1.0f});
			}
		}
	}
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
