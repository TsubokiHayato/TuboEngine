#pragma once
#include "IStageState.h"
#include "Vector3.h"
#include <chrono>
#include <vector>
#include <memory>
#include "Sprite.h"


class StageScene;

/// Playing状態 （ステージプレイ中の状態）///
 class StagePlayingState : public IStageState {
	public:
	void Enter(StageScene* scene) override;
	void Update(StageScene* scene) override;
	void Exit(StageScene* scene) override;
	void Object3DDraw(StageScene* scene) override;
	void SpriteDraw(StageScene* scene) override;
	void ImGuiDraw(StageScene* scene) override;
	void ParticleDraw(StageScene* scene) override;

private:
	std::unique_ptr<Sprite> pauseGuideSprite_ = nullptr; // 追加: ポーズ誘導UI


 };
