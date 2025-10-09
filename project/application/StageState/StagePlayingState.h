#pragma once
#include "IStageState.h"
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
 };
