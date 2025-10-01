#include "StageStateManager.h"
#include"StageScene.h"

void StageStateManager::Initialize(StageScene* scene) {
    states_[State::Ready]        = std::make_unique<StageReadyState>();
    states_[State::Playing]      = std::make_unique<StagePlayingState>();
    states_[State::StageClear]   = std::make_unique<StageClearState>();
    states_[State::RewardSelect] = std::make_unique<RewardSelectState>();
    states_[State::Boss]         = std::make_unique<BossState>();
    states_[State::GameClear]    = std::make_unique<GameClearState>();
    states_[State::GameOver]     = std::make_unique<GameOverState>();

    // 各ステートのInitializeを呼ぶ（必要なら）
    for (auto& [state, ptr] : states_) {
        ptr->Enter(scene);
    }

    // 最初のステートをセット
    currentState_ = states_[State::Ready].get();
}

void StageStateManager::Update(StageScene* scene) {
	if (currentState_) {
		currentState_->Update(scene);
	}
}

void StageStateManager::ChangeState(IStageState* newState, StageScene* scene) {
	if (currentState_)
		currentState_->Exit(scene);
	currentState_ = std::move(newState);
	if (currentState_)
		currentState_->Enter(scene);
}
