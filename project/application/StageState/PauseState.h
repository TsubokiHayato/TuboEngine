#pragma once

#include "IStageState.h"
#include <memory>

class Sprite;

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
	std::unique_ptr<Sprite> background_; // 追加: 背景用の半透明グレー
	std::unique_ptr<Sprite> blackout_;
	std::unique_ptr<Sprite> cursor_;
	std::unique_ptr<Sprite> resumeText_;
	std::unique_ptr<Sprite> restartText_;
	std::unique_ptr<Sprite> titleText_;
};
