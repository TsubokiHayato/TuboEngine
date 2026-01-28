#include "PauseState.h"

#include "ImGuiManager.h"
#include "Input.h"
#include "Sprite.h"
#include "StageScene.h"
#include "TextureManager.h"
#include <cmath>

namespace {
constexpr float kScreenW = 1280.0f;
constexpr float kScreenH = 720.0f;
constexpr float kItemX = 1280.0f / 3;
constexpr float kResumeY = 300.0f;
constexpr float kRestartY = 390.0f;
constexpr float kTitleY = 480.0f;

// ---- ImGui調整用（実行中に変更可能） ----
float gItemX = 1280.0f / 3.0f;
float gResumeY = 300.0f;
float gRestartY = 390.0f;
float gTitleY = 480.0f;
float gCursorOffsetX = 16.0f; // X方向オフセット
float gCursorOffsetY = 40.0f; // 追加: Y方向オフセット
float gCursorSize = 26.0f;

// 追加: 背景（画面全体の半透明グレー）
float gBackgroundAlpha = 0.55f;

// PoseUI/Pose.png はサイズ固定（調整しない）。位置だけImGui対応。
float gPoseX = kScreenW * 0.5f; // X軸：真ん中
float gPoseY = 100.0f;          // Y軸：上側（上からのマージン）

// ---- Poseアニメ（位置/アルファのみ。サイズは触らない） ----
bool gPoseAnimEnabled = true;
float gPoseAnimTime = 0.0f;
float gPoseFloatAmp = 40.0f;  // 上下振幅(px)
float gPoseFloatSpeed = 1.6f; // ふわふわ速度
float gPoseFadeSpeed = 6.0f;  // フェードイン速度
float gPoseAlpha = 0.0f;      // 現在アルファ（0..1）
// 追加: 回転・カラー
float gPoseRotDegAmp = 3.0f;                  // 回転振幅(度)
float gPoseRotSpeed = 1.2f;                   // 回転速度
float gPoseBaseColor[3] = {1.0f, 1.0f, 1.0f}; // RGB
bool gPoseColorPulse = false;
float gPoseColorPulseAmp = 0.15f;
float gPoseColorPulseSpeed = 2.0f;
} // namespace

void PauseState::Enter(StageScene* /*scene*/) {
	selected_ = 0;

	// アニメ初期化（ポーズに入った瞬間はフェードイン）
	gPoseAnimTime = 0.0f;
	gPoseAlpha = 0.0f;

	// Simple textures (reuse existing ones).
	TextureManager::GetInstance()->LoadTexture("barrier.png");
	TextureManager::GetInstance()->LoadTexture("TitleUI/Start.png");
	TextureManager::GetInstance()->LoadTexture("TitleUI/Exit.png");
	TextureManager::GetInstance()->LoadTexture("PoseUI/Pose.png");
	TextureManager::GetInstance()->LoadTexture("PoseUI/ReturnGame.png");
	TextureManager::GetInstance()->LoadTexture("PoseUI/ReStart.png");
	TextureManager::GetInstance()->LoadTexture("PoseUI/ReturnTitle.png");

	// 追加: 背景（半透明グレー）
	background_ = std::make_unique<Sprite>();
	background_->Initialize("barrier.png");
	background_->SetAnchorPoint({0.0f, 0.0f});
	background_->SetSize({kScreenW, kScreenH});
	background_->SetPosition({0.0f, 0.0f});
	background_->SetColor({0.0f, 0.0f, 0.0f, gBackgroundAlpha});
	background_->Update();

	blackout_ = std::make_unique<Sprite>();
	blackout_->Initialize("PoseUI/Pose.png");
	blackout_->SetAnchorPoint({0.5f, 0.0f});    // Xは中央、Yは上揃え
	blackout_->SetGetIsAdjustTextureSize(true); // テクスチャ実サイズ（サイズ調整しない）
	blackout_->SetPosition({gPoseX, gPoseY});
	blackout_->SetColor({1.0f, 1.0f, 1.0f, gPoseAlpha});
	blackout_->Update();

	// Menu labels
	resumeText_ = std::make_unique<Sprite>();
	resumeText_->Initialize("PoseUI/ReturnGame.png");
	resumeText_->SetAnchorPoint({0.0f, 0.5f});
	resumeText_->SetGetIsAdjustTextureSize(true);
	resumeText_->SetPosition({gItemX, gResumeY});
	resumeText_->Update();

	restartText_ = std::make_unique<Sprite>();
	restartText_->Initialize("PoseUI/ReStart.png");
	restartText_->SetAnchorPoint({0.0f, 0.5f});
	restartText_->SetGetIsAdjustTextureSize(true);
	restartText_->SetPosition({gItemX, gRestartY});
	restartText_->Update();

	titleText_ = std::make_unique<Sprite>();
	titleText_->Initialize("PoseUI/ReturnTitle.png");
	titleText_->SetAnchorPoint({0.0f, 0.5f});
	titleText_->SetGetIsAdjustTextureSize(true);
	titleText_->SetPosition({gItemX, gTitleY});
	titleText_->Update();

	// Cursor: small white rectangle
	cursor_ = std::make_unique<Sprite>();
	cursor_->Initialize("barrier.png");
	cursor_->SetAnchorPoint({1.0f, 0.5f});
	cursor_->SetSize({gCursorSize, gCursorSize});
	cursor_->SetColor({1.0f, 1.0f, 1.0f, 0.9f});
	UpdateCursor();
	cursor_->Update();
}

void PauseState::Update(StageScene* scene) {
	auto* input = Input::GetInstance();

	// 既にシーンチェンジ演出中なら、完了待ち
	if (scene && scene->GetIsRequestSceneChange()) {
		if (scene->GetSceneChangeAnimation() && scene->GetSceneChangeAnimation()->IsFinished()) {
			// 演出完了でタイトルへ
			scene->SetIsRequestSceneChange(false);
			scene->ChangeNextScene(TITLE);
		}
		return;
	}

	// Toggle resume
	if (input->TriggerKey(DIK_ESCAPE)) {
		if (scene && scene->GetStageStateManager()) {
			scene->GetStageStateManager()->ChangeState(StageType::Playing, scene);
		}
		return;
	}

	// Navigate
	if (input->TriggerKey(DIK_UP) || input->TriggerKey(DIK_W)) {
		selected_ = (selected_ + kItemCount - 1) % kItemCount;
		UpdateCursor();
	}
	if (input->TriggerKey(DIK_DOWN) || input->TriggerKey(DIK_S)) {
		selected_ = (selected_ + 1) % kItemCount;
		UpdateCursor();
	}

	// Decide
	if (input->TriggerKey(DIK_RETURN) || input->TriggerKey(DIK_SPACE)) {
		if (selected_ == 0) {
			if (scene && scene->GetStageStateManager()) {
				scene->GetStageStateManager()->ChangeState(StageType::Playing, scene);
			}
			return;
		}
		if (selected_ == 1) {
			if (scene && scene->GetStageStateManager()) {
				scene->GetStageStateManager()->ChangeState(StageType::Ready, scene);
			}
			return;
		}
		if (selected_ == 2) {
			// タイトルへ戻る：シーンチェンジアニメーションを挟む
			if (scene && scene->GetSceneChangeAnimation()) {
				// アニメがアイドル（Finished）なら開始
				if (scene->GetSceneChangeAnimation()->IsFinished()) {
					scene->GetSceneChangeAnimation()->SetPhase(SceneChangeAnimation::Phase::Appearing);
					scene->SetIsRequestSceneChange(true);
					return;
				}
			}
			// フォールバック（アニメが無い場合）
			if (scene) {
				scene->ChangeNextScene(TITLE);
			}
			return;
		}
	}

	// ---- Poseアニメ更新（dtは60fps前提） ----
	const float dt = 1.0f / 60.0f;
	gPoseAnimTime += dt;
	if (gPoseAnimEnabled) {
		// フェードイン
		gPoseAlpha += gPoseFadeSpeed * dt;
		if (gPoseAlpha > 1.0f)
			gPoseAlpha = 1.0f;
	} else {
		gPoseAlpha = 1.0f;
	}
	const float floatY = gPoseAnimEnabled ? (std::sinf(gPoseAnimTime * gPoseFloatSpeed) * gPoseFloatAmp) : 0.0f;
	const float rotDeg = gPoseAnimEnabled ? (std::sinf(gPoseAnimTime * gPoseRotSpeed) * gPoseRotDegAmp) : 0.0f;
	const float rotRad = rotDeg * 3.14159265f / 180.0f;
	float pulse = 0.0f;
	if (gPoseAnimEnabled && gPoseColorPulse) {
		pulse = std::sinf(gPoseAnimTime * gPoseColorPulseSpeed) * gPoseColorPulseAmp;
	}

	// 背景（半透明グレー）
	if (background_) {
		background_->SetSize({kScreenW, kScreenH});
		background_->SetPosition({0.0f, 0.0f});
		background_->SetColor({0.0f, 0.0f, 0.0f, gBackgroundAlpha});
		background_->Update();
	}

	// ImGui調整内容を反映
	if (blackout_) {
		blackout_->SetPosition({gPoseX, gPoseY + floatY});
		blackout_->SetRotation(rotRad);
		Vector4 col = blackout_->GetColor();
		col.x = gPoseBaseColor[0] + pulse;
		col.y = gPoseBaseColor[1] + pulse;
		col.z = gPoseBaseColor[2] + pulse;
		col.w = gPoseAlpha;
		blackout_->SetColor(col);
	}
	if (resumeText_)
		resumeText_->SetPosition({gItemX, gResumeY});
	if (restartText_)
		restartText_->SetPosition({gItemX, gRestartY});
	if (titleText_)
		titleText_->SetPosition({gItemX, gTitleY});
	if (cursor_)
		cursor_->SetSize({gCursorSize, gCursorSize});
	UpdateCursor();

	if (blackout_)
		blackout_->Update();
	if (resumeText_)
		resumeText_->Update();
	if (restartText_)
		restartText_->Update();
	if (titleText_)
		titleText_->Update();
	if (cursor_)
		cursor_->Update();
}

void PauseState::Exit(StageScene* /*scene*/) {
	background_.reset();
	blackout_.reset();
	cursor_.reset();
	resumeText_.reset();
	restartText_.reset();
	titleText_.reset();
}

void PauseState::Object3DDraw(StageScene* scene) {
	// Draw the current scene behind the pause UI
	if (!scene)
		return;

	if (scene->GetSkyDome())
		scene->GetSkyDome()->Draw();
	if (auto& tile = scene->GetTile())
		tile->Draw();
	for (auto& block : scene->GetBlocks()) {
		if (block)
			block->Draw();
	}
	if (scene->GetPlayer())
		scene->GetPlayer()->Draw();
	for (auto& enemy : scene->GetEnemies()) {
		if (enemy)
			enemy->Draw();
	}

	// preview stages (if any)
	auto& stages = scene->GetStageInstances();
	for (size_t i = 1; i < stages.size(); ++i) {
		auto& st = stages[i];
		if (!st.visible)
			continue;
		if (st.tile)
			st.tile->Draw();
		for (auto& b : st.blocks)
			if (b)
				b->Draw();
		for (auto& e : st.enemies)
			if (e)
				e->Draw();
	}
}

void PauseState::SpriteDraw(StageScene* scene) {
	// Draw underlying sprite elements if needed (reticle etc.)
	if (scene && scene->GetPlayer()) {
		scene->GetPlayer()->ReticleDraw();
	}

	// 追加: 背景→UIの順で描画
	if (background_)
		background_->Draw();
	if (blackout_)
		blackout_->Draw();
	if (resumeText_)
		resumeText_->Draw();
	if (restartText_)
		restartText_->Draw();
	if (titleText_)
		titleText_->Draw();
	if (cursor_)
		cursor_->Draw();
}

void PauseState::ImGuiDraw(StageScene* /*scene*/) {
#ifdef USE_IMGUI
	ImGui::Begin("Pause UI");
	ImGui::DragFloat("ItemX", &gItemX, 1.0f, 0.0f, 1280.0f);
	ImGui::DragFloat("ResumeY", &gResumeY, 1.0f, 0.0f, 720.0f);
	ImGui::DragFloat("RestartY", &gRestartY, 1.0f, 0.0f, 720.0f);
	ImGui::DragFloat("TitleY", &gTitleY, 1.0f, 0.0f, 720.0f);
	ImGui::DragFloat("CursorOffsetX", &gCursorOffsetX, 1.0f, -300.0f, 300.0f);
	ImGui::DragFloat("CursorOffsetY", &gCursorOffsetY, 1.0f, -300.0f, 300.0f);
	ImGui::DragFloat("CursorSize", &gCursorSize, 1.0f, 1.0f, 200.0f);
	ImGui::Separator();
	ImGui::Text("Background (screen overlay)");
	ImGui::DragFloat("BackgroundAlpha", &gBackgroundAlpha, 0.01f, 0.0f, 1.0f);
	ImGui::Separator();
	ImGui::Text("Pose (PoseUI/Pose.png) position + animation");
	ImGui::DragFloat("PoseX", &gPoseX, 1.0f, -2000.0f, 2000.0f);
	ImGui::DragFloat("PoseY", &gPoseY, 1.0f, -2000.0f, 2000.0f);
	ImGui::Checkbox("PoseAnimEnabled", &gPoseAnimEnabled);
	ImGui::DragFloat("PoseFloatAmp", &gPoseFloatAmp, 0.1f, 0.0f, 200.0f);
	ImGui::DragFloat("PoseFloatSpeed", &gPoseFloatSpeed, 0.01f, 0.0f, 20.0f);
	ImGui::DragFloat("PoseFadeSpeed", &gPoseFadeSpeed, 0.1f, 0.0f, 50.0f);
	ImGui::DragFloat("PoseRotDegAmp", &gPoseRotDegAmp, 0.1f, 0.0f, 45.0f);
	ImGui::DragFloat("PoseRotSpeed", &gPoseRotSpeed, 0.01f, 0.0f, 20.0f);
	ImGui::ColorEdit3("PoseBaseColor", gPoseBaseColor);
	ImGui::Checkbox("PoseColorPulse", &gPoseColorPulse);
	ImGui::DragFloat("PoseColorPulseAmp", &gPoseColorPulseAmp, 0.01f, 0.0f, 1.0f);
	ImGui::DragFloat("PoseColorPulseSpeed", &gPoseColorPulseSpeed, 0.01f, 0.0f, 20.0f);
	ImGui::Text("PoseAlpha: %.2f", gPoseAlpha);
	ImGui::End();
#endif
}

void PauseState::ParticleDraw(StageScene* /*scene*/) {
	// No particles in pause state
}

void PauseState::UpdateCursor() {
	if (!cursor_)
		return;
	float y = gResumeY;
	if (selected_ == 1)
		y = gRestartY;
	else if (selected_ == 2)
		y = gTitleY;
	// X/Y方向のオフセットを適用
	cursor_->SetPosition({gItemX - gCursorOffsetX, y + gCursorOffsetY});
}
