#pragma once

#include "StageState/IStageState.h"
#include <memory>

#include "Sprite.h"

// PauseState:
// - Overlay pause menu shown during gameplay.
// - ESC toggles resume.
// - Provide simple options: Resume / Restart / Title.
class PauseState final : public IStageState {
public:
	void Enter(StageScene* scene) override;
	void Update(StageScene* scene) override;
	void Exit(StageScene* scene) override;
	void Object3DDraw(StageScene* scene) override;
	void SpriteDraw(StageScene* scene) override;
	void ImGuiDraw(StageScene* scene) override;
	void ParticleDraw(StageScene* scene) override;

private:
	void UpdateCursor();

	static constexpr int kItemCount = 3;
	int selected_ = 0; // 0: Resume, 1: Restart, 2: Title
	std::unique_ptr<TuboEngine::Sprite> background_; 
	std::unique_ptr<TuboEngine::Sprite> blackout_;
	std::unique_ptr<TuboEngine::Sprite> cursor_;
	std::unique_ptr<TuboEngine::Sprite> resumeText_;
	std::unique_ptr<TuboEngine::Sprite> restartText_;
	std::unique_ptr<TuboEngine::Sprite> titleText_;
};
