#pragma once
#include "IStageState.h"
class StageScene;

class BossState :public IStageState {
public:
	void Enter(StageScene* scene) override;
	void Update(StageScene* scene) override;
	void Exit(StageScene* scene) override;
};
