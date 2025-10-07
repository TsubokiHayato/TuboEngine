#pragma once
#include "IStageState.h"
class StageScene;
/// Ready状態 （ステージ開始前の準備状態）///

///---------------------------------------------------------
/// 各キャラクターの初期化やマップチップの読み込みを行う
/// 及び,出現アニメーションを再生するState
/// ---------------------------------------------------------
 class StageReadyState : public IStageState {

 public:
	void Enter(StageScene* scene) override;
	void Update(StageScene* scene) override;
	void Exit(StageScene* scene) override;
	void Object3DDraw(StageScene* scene) override;
	void SpriteDraw(StageScene* scene) override;
	void ImGuiDraw(StageScene* scene) override;
	void ParticleDraw(StageScene* scene) override;

 };
