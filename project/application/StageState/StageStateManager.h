#pragma once
#include "BossState.h"
#include "GameClearState.h"
#include "GameOverState.h"
#include "IStageState.h"
#include "RewardSelectState.h"
#include "StageClearState.h"
#include "StagePlayingState.h"
#include "StageReadyState.h"
#include "TutorialState.h"
#include "StageType.h"
#include <map>
#include <memory>
class StageStateManager {

	///---------------------------------------------------------
	///				メンバ関数
	///---------------------------------------------------------
public:
	void Initialize(StageScene* scene);
	void Update(StageScene* scene);
	void Object3DDraw(StageScene* scene);
	void SpriteDraw(StageScene* scene);
	void ParticleDraw(StageScene* scene);
	void DrawImGui(StageScene* scene);

	void ChangeState(StageType nextType, StageScene* scene);
	void NextStage() { ++stageNumber_; }

	/// -----------------------------------------------------
	///				ゲッター・セッター
	/// -----------------------------------------------------
public:
	void SetState(StageType state) { state_ = state; }
	StageType GetState() const { return state_; }

	int GetStageNumber() const { return stageNumber_; }
	void SetStageNumber(int num) { stageNumber_ = num; }

	///---------------------------------------------------------
	///				メンバ変数
	///---------------------------------------------------------
private:
	StageType state_ = StageType::Ready;
	StageType pendingState_ = static_cast<StageType>(-1); // 未設定状態を-1で表現
	int stageNumber_ = 1;
	std::map<StageType, std::unique_ptr<IStageState>> states_;
	IStageState* currentState_;
};
