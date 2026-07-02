#pragma once
#include "GameClear/GameClearState.h"
#include "GameOver/GameOverState.h"
#include "StageState/IStageState.h"
#include "StageClear/StageClearState.h"
#include "StagePlaying/StagePlayingState.h"
#include "StageReady/StageReadyState.h"
#include "Tutorial/TutorialState.h"
#include "Pause/PauseState.h"
#include "StageType.h"
#include <map>
#include <memory>



/// <summary>
/// ステージ内ステート（Ready/Playing/Clear など）の切り替えと、更新・描画の振り分けを行うクラス。
/// </summary>
class StageStateManager {

	///---------------------------------------------------------
	///				メンバ関数
	///---------------------------------------------------------
public:
	/// <summary>
	/// 初期化処理。
	/// </summary>
	void Initialize(StageScene* scene);
	/// <summary>
	/// 更新処理。
	/// </summary>
	void Update(StageScene* scene);
	/// <summary>
	/// 3Dオブジェクト描画。
	/// </summary>
	void Object3DDraw(StageScene* scene);
	/// <summary>
	/// スプライト描画。
	/// </summary>
	void SpriteDraw(StageScene* scene);
	/// <summary>
	/// パーティクル描画。
	/// </summary>
	void ParticleDraw(StageScene* scene);
	/// <summary>
	/// ImGuiによるデバッグ表示。
	/// </summary>
	void DrawImGui(StageScene* scene);

	/// <summary>
	/// ステートを切り替える。
	/// </summary>
	void ChangeState(StageType nextType, StageScene* scene);
	/// <summary>
	/// ステージ番号を進める。
	/// </summary>
	void NextStage() { ++stageNumber_; }

	/// -----------------------------------------------------
	///				ゲッター・セッター
	/// -----------------------------------------------------
public:
	/// <summary>
	/// State の取得・設定。
	/// </summary>
	void SetState(StageType state) { state_ = state; }
	StageType GetState() const { return state_; }

	/// <summary>
	/// StageNumber の取得・設定。
	/// </summary>
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


