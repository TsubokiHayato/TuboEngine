#include "StageStateManager.h"
#include"StageScene.h"
#include "ImGuiManager.h"
#include"Input.h"

void StageStateManager::Initialize(StageScene* scene) {
    states_[StageType::Ready]        = std::make_unique<StageReadyState>();
    states_[StageType::Playing]      = std::make_unique<StagePlayingState>();
    states_[StageType::Pause]        = std::make_unique<PauseState>();
    states_[StageType::StageClear]   = std::make_unique<StageClearState>();
    states_[StageType::RewardSelect] = std::make_unique<RewardSelectState>();
    states_[StageType::Boss]         = std::make_unique<BossState>();
    states_[StageType::GameClear]    = std::make_unique<GameClearState>();
    states_[StageType::GameOver]     = std::make_unique<GameOverState>();

    // 新規: TutorialState（他Stateと合併せず、独立して存在させる）
    states_[StageType::Tutorial]     = std::make_unique<TutorialState>();

    // 最初のステートをセットしてEnterのみ呼ぶ
    state_ = StageType::Ready;
	currentState_ = states_[StageType::Ready].get();
    if (currentState_) {
        currentState_->Enter(scene);
    }
}

void StageStateManager::Update(StageScene* scene) {
    if (currentState_) {
        currentState_->Update(scene);
    }


    // デバッグ用：ImGuiからのステート遷移要求を処理
    if (pendingState_ != static_cast<StageType>(-1)) {
        ChangeState(pendingState_, scene);
        pendingState_ = static_cast<StageType>(-1);
    }
}

void StageStateManager::Object3DDraw(StageScene* scene) {
	if (currentState_) {
		currentState_->Object3DDraw(scene);
	}
}

void StageStateManager::SpriteDraw(StageScene* scene) {
    if (currentState_) {
        currentState_->SpriteDraw(scene);
    }
}

void StageStateManager::ParticleDraw(StageScene* scene) {
    if (currentState_) {
        currentState_->ParticleDraw(scene);
    }
}

void StageStateManager::ChangeState(StageType nextType, StageScene* scene) {
    // 次のステートが生成済みかチェック
    auto it = states_.find(nextType);
    if (it == states_.end()) {
        return;
    }

    if (currentState_) {
        currentState_->Exit(scene);
    }

    // 重要: ステート遷移直後に「Enterで使うTrigger判定」が誤爆しないようにする
    // (例: Pauseメニュー決定にSPACEを使うと、次ステートで回避/ダッシュが即発動する)
    if (Input::GetInstance()) {
        Input::GetInstance()->FlushTriggers();
    }

    currentState_ = it->second.get();
    state_ = nextType;
    if (currentState_) {
        currentState_->Enter(scene);
    }
}

void StageStateManager::DrawImGui(StageScene* scene) {

#ifdef USE_IMGUI
    // 各々のStateのImGui描画
    if (currentState_) {
        currentState_->ImGuiDraw(scene);
	}

    ImGui::Begin("Stage State Manager");
    // State表示
    ImGui::Text("Current State: ");
    switch (state_) {
        case StageType::Ready:
            ImGui::Text("Ready");
            break;
        case StageType::Playing:
            ImGui::Text("Playing");
            break;
        case StageType::Pause:
            ImGui::Text("Pause");
            break;
        case StageType::StageClear:
            ImGui::Text("StageClear");
            break;
        case StageType::RewardSelect:
            ImGui::Text("RewardSelect");
            break;
        case StageType::Boss:
            ImGui::Text("Boss");
            break;
        case StageType::GameClear:
            ImGui::Text("GameClear");
            break;
        case StageType::GameOver:
            ImGui::Text("GameOver");
            break;
        case StageType::Tutorial:
            ImGui::Text("Tutorial");
            break;
        default:
            ImGui::Text("Unknown State");
            break;
    }
    // ステージ番号表示
    ImGui::Text("Stage Number: %d", stageNumber_);

    // StateChangeボタン
    if (ImGui::Button("Next Stage")) {
        NextStage();
    }
	if (ImGui::Button("Ready")) {
		pendingState_ = StageType::Ready;
	}
	if (ImGui::Button("Playing")) {
		pendingState_ = StageType::Playing;
	}
	if (ImGui::Button("Pause")) {
		pendingState_ = StageType::Pause;
	}
	if (ImGui::Button("StageClear")) {
		pendingState_ = StageType::StageClear;
	}
	if (ImGui::Button("RewardSelect")) {
		pendingState_ = StageType::RewardSelect;
	}
	if (ImGui::Button("Boss")) {
		pendingState_ = StageType::Boss;
	}
	if (ImGui::Button("GameClear")) {
		pendingState_ = StageType::GameClear;
	}
	if (ImGui::Button("GameOver")) {
		pendingState_ = StageType::GameOver;
	}
	if (ImGui::Button("Tutorial")) {
		pendingState_ = StageType::Tutorial;
	}

    ImGui::End();
#endif // USE_IMGUI
}
