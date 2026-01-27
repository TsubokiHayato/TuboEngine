#include "PauseState.h"

#include "StageScene.h"
#include "Input.h"
#include "TextureManager.h"
#include "Sprite.h"

namespace {
	constexpr float kScreenW = 1280.0f;
	constexpr float kScreenH = 720.0f;
	constexpr float kItemX = 640.0f;
	constexpr float kResumeY = 300.0f;
	constexpr float kRestartY = 390.0f;
	constexpr float kTitleY = 480.0f;
}

void PauseState::Enter(StageScene* /*scene*/) {
	selected_ = 0;

	// Simple textures (reuse existing ones).
	TextureManager::GetInstance()->LoadTexture("barrier.png");
	TextureManager::GetInstance()->LoadTexture("TitleUI/Start.png");
	TextureManager::GetInstance()->LoadTexture("TitleUI/Exit.png");
	TextureManager::GetInstance()->LoadTexture("PoseUI/ReturnGame.png");
	TextureManager::GetInstance()->LoadTexture("PoseUI/ReStart.png");
	TextureManager::GetInstance()->LoadTexture("PoseUI/ReturnTitle.png");

	blackout_ = std::make_unique<Sprite>();
	blackout_->Initialize("barrier.png");
	blackout_->SetSize({kScreenW, kScreenH});
	blackout_->SetColor({0.0f, 0.0f, 0.0f, 0.55f});
	blackout_->SetPosition({0.0f, 0.0f});
	blackout_->Update();

	// Menu labels
	resumeText_ = std::make_unique<Sprite>();
	resumeText_->Initialize("PoseUI/ReturnGame.png");
	resumeText_->SetAnchorPoint({0.5f, 0.5f});
	resumeText_->SetGetIsAdjustTextureSize(true);
	resumeText_->SetPosition({kItemX, kResumeY});
	resumeText_->Update();

	restartText_ = std::make_unique<Sprite>();
	restartText_->Initialize("PoseUI/ReStart.png");
	restartText_->SetAnchorPoint({0.5f, 0.5f});
	restartText_->SetGetIsAdjustTextureSize(true);
	restartText_->SetPosition({kItemX, kRestartY});
	restartText_->Update();

	titleText_ = std::make_unique<Sprite>();
	titleText_->Initialize("PoseUI/ReturnTitle.png");
	titleText_->SetAnchorPoint({0.5f, 0.5f});
	titleText_->SetGetIsAdjustTextureSize(true);
	titleText_->SetPosition({kItemX, kTitleY});
	titleText_->Update();

	// Cursor: small white rectangle
	cursor_ = std::make_unique<Sprite>();
	cursor_->Initialize("barrier.png");
	cursor_->SetAnchorPoint({0.5f, 0.5f});
	cursor_->SetSize({26.0f, 26.0f});
	cursor_->SetColor({1.0f, 1.0f, 1.0f, 0.9f});
	UpdateCursor();
	cursor_->Update();
}

void PauseState::Update(StageScene* scene) {
	auto* input = Input::GetInstance();

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
			if (scene) {
				scene->ChangeNextScene(TITLE);
			}
			return;
		}
	}

	if (blackout_) blackout_->Update();
	if (resumeText_) resumeText_->Update();
	if (restartText_) restartText_->Update();
	if (titleText_) titleText_->Update();
	if (cursor_) cursor_->Update();
}

void PauseState::Exit(StageScene* /*scene*/) {
	blackout_.reset();
	cursor_.reset();
	resumeText_.reset();
	restartText_.reset();
	titleText_.reset();
}

void PauseState::Object3DDraw(StageScene* scene) {
	// Draw the current scene behind the pause UI
	if (!scene) return;

	if (scene->GetSkyDome()) scene->GetSkyDome()->Draw();
	if (auto& tile = scene->GetTile()) tile->Draw();
	for (auto& block : scene->GetBlocks()) {
		if (block) block->Draw();
	}
	if (scene->GetPlayer()) scene->GetPlayer()->Draw();
	for (auto& enemy : scene->GetEnemies()) {
		if (enemy) enemy->Draw();
	}

	// preview stages (if any)
	auto& stages = scene->GetStageInstances();
	for (size_t i = 1; i < stages.size(); ++i) {
		auto& st = stages[i];
		if (!st.visible) continue;
		if (st.tile) st.tile->Draw();
		for (auto& b : st.blocks) if (b) b->Draw();
		for (auto& e : st.enemies) if (e) e->Draw();
	}
}

void PauseState::SpriteDraw(StageScene* scene) {
	// Draw underlying sprite elements if needed (reticle etc.)
	if (scene && scene->GetPlayer()) {
		scene->GetPlayer()->ReticleDraw();
	}

	if (blackout_) blackout_->Draw();
	if (resumeText_) resumeText_->Draw();
	if (restartText_) restartText_->Draw();
	if (titleText_) titleText_->Draw();
	if (cursor_) cursor_->Draw();
}

void PauseState::ImGuiDraw(StageScene* /*scene*/) {
}

void PauseState::ParticleDraw(StageScene* scene) {
	// Keep particles frozen by not updating them; optionally still draw existing.
	if (!scene) return;
	for (auto& enemy : scene->GetEnemies()) {
		if (!enemy) continue;
		enemy->ParticleDraw();
	}
}

void PauseState::UpdateCursor() {
	if (!cursor_) return;
	float y = kResumeY;
	if (selected_ == 1) y = kRestartY;
	else if (selected_ == 2) y = kTitleY;
	cursor_->SetPosition({kItemX - 170.0f, y});
}
