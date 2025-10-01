#pragma once
#include "IStageState.h"
class StageScene;
/// Ready状態 （ステージ開始前の準備状態）///
 class StageReadyState : public IStageState {

 public:
	void Enter(StageScene* scene) override;
	void Update(StageScene* scene) override;
	void Exit(StageScene* scene) override;
 };
