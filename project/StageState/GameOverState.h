#pragma once
class StageScene; 
#include "IStageState.h"

class GameOverState : public IStageState
{
public:
    void Enter(StageScene* scene) override;
    void Update(StageScene* scene) override;
    void Exit(StageScene* scene) override;
	void Object3DDraw(StageScene* scene) override;
	void SpriteDraw(StageScene* scene) override;
	void ImGuiDraw(StageScene* scene) override;
	void ParticleDraw(StageScene* scene) override;
};
