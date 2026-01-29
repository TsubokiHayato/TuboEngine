#pragma once
#include "IStageState.h"
#include "Vector3.h"
#include <chrono>
#include <memory>

class Sprite;
class StageScene;

/// <summary>
/// ステージ開始前の準備状態（Ready）
/// ・配置は即座に行う
/// ・READY/START!! 表示だけ行ってから Playing に遷移する
/// </summary>
class StageReadyState : public IStageState {
public:
	void Enter(StageScene* scene) override;
	void Update(StageScene* scene) override;
	void Exit(StageScene* scene) override;
	void Object3DDraw(StageScene* scene) override;
	void SpriteDraw(StageScene* scene) override;
	void ImGuiDraw(StageScene* scene) override;
	void ParticleDraw(StageScene* scene) override;

private:
	std::chrono::steady_clock::time_point prevTime_{};

	enum class Phase {
		Ready,
		Start,
		Done,
	};
	Phase phase_ = Phase::Ready;
	float phaseTimer_ = 0.0f;
	float readyDuration_ = 0.8f;
	float startDuration_ = 0.6f;

	std::unique_ptr<Sprite> readySprite_;
	std::unique_ptr<Sprite> startSprite_;
};
