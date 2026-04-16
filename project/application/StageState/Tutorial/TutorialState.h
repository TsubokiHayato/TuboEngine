#pragma once
#include "StageState/IStageState.h"
#include <string>
#include <memory>
#include"Sprite.h"

// TutorialState:
// - 他Stateと合併しない「独立したState」
// - ReadyStateで用意したものを使わず、チュートリアル用CSVからこのState内で生成する
class TutorialState final : public IStageState {
public:
	void Enter(StageScene* scene) override;
	void Update(StageScene* scene) override;
	void Exit(StageScene* scene) override;
	void Object3DDraw(StageScene* scene) override;
	void SpriteDraw(StageScene* scene) override;
	void ImGuiDraw(StageScene* scene) override;
	void ParticleDraw(StageScene* scene) override;

private:
	void BuildTutorialStage(StageScene* scene);
	void InitializeTutorialSprites();

	float elapsed_ = 0.0f;
	int step_ = 0;
	bool built_ = false;
	std::string tutorialCsvPath_ = "Resources/Tutorial.csv";

	// Tutorial UI sprites (text baked into textures)
	std::unique_ptr<TuboEngine::Sprite> tutorialHeaderSprite_;
	std::unique_ptr<TuboEngine::Sprite> tutorialMoveSprite_;
	std::unique_ptr<TuboEngine::Sprite> tutorialAttackSprite_;
	std::unique_ptr<TuboEngine::Sprite> tutorialDashSprite_;
	std::unique_ptr<TuboEngine::Sprite> howtoNextSprite_;
	std::unique_ptr<TuboEngine::Sprite> titleBackSprite_;
};
