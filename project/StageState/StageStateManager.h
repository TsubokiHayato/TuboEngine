#pragma once
#include "IStageState.h"
#include "StageClearState.h"
#include "StagePlayingState.h"
#include "StageReadyState.h"
#include "RewardSelectState.h"
#include "BossState.h"
#include "GameClearState.h"
#include "GameOverState.h"
#include <map>
#include <memory>

class StageStateManager {

	///---------------------------------------------------------
	///				メンバ関数
	///---------------------------------------------------------
public:
	enum class State { Ready, Playing, StageClear, RewardSelect, Boss, GameClear, GameOver };

	void Initialize(StageScene* scene);
	void Update(StageScene* scene);
	void ChangeState(IStageState* newState, StageScene* scene);
	void NextStage() { ++stageNumber_; }

	/// -----------------------------------------------------
	///				ゲッター・セッター
	/// -----------------------------------------------------
public:
	void SetState(State state) { state_ = state; }
	State GetState() const { return state_; }

	int GetStageNumber() const { return stageNumber_; }
	void SetStageNumber(int num) { stageNumber_ = num; }

	const int GetFinalStageNumber() const { return FINAL_STAGE_NUM; }

	///---------------------------------------------------------
	///				メンバ変数
	///---------------------------------------------------------
private:
	State state_ = State::Ready;
	int stageNumber_ = 1;
	const int FINAL_STAGE_NUM = 3;
	std::map<State, std::unique_ptr<IStageState>> states_;
	IStageState* currentState_;
};
