#pragma once
#include "IStageState.h"
#include "Vector3.h"
#include <chrono>
#include <vector>
#include <memory>
#include "Sprite.h"

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

	private:
	std::unique_ptr<Sprite> restartSprite_ = nullptr;

	private:
	    bool started_ = false;
	    float timer_ = 0.0f;
	    float durationSec_ = 1.2f; // 演出時間：終了後にCLEARへ

 };
