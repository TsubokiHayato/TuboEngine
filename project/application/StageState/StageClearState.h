#pragma once
#include "IStageState.h"
class StageScene;
    /// StageClear状態 （ステージクリア後の状態）///
 class StageClearState : public IStageState {
	public:
	void Enter(StageScene* scene) override;
	void Update(StageScene* scene) override;
	void Exit(StageScene* scene) override;
	void Object3DDraw(StageScene* scene) override;
	void SpriteDraw(StageScene* scene) override;
	void ImGuiDraw(StageScene* scene) override;
	void ParticleDraw(StageScene* scene) override;
 };
