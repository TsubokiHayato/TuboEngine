#pragma once
#include "StageState/IStageState.h"
#include "Vector3.h"
#include <chrono>

class StageTransitionState : public IStageState {
 public:
 	void Enter(StageScene* scene) override;
 	void Update(StageScene* scene) override;
 	void Exit(StageScene* scene) override;
 	void Object3DDraw(StageScene* scene) override;
 	void SpriteDraw(StageScene* scene) override;
 	void ImGuiDraw(StageScene* scene) override;
 	void ParticleDraw(StageScene* scene) override;

 private:
 	std::chrono::steady_clock::time_point startTime_{};
 	float durationSec_ = 1.5f; // イージング移動時間
 	float jumpHeight_ = 8.0f;  // ジャンプの最大高さ（壁を飛び越える）
 	TuboEngine::Math::Vector3 startPos_{};
 	TuboEngine::Math::Vector3 targetPos_{};
 	bool initialized_ = false;

 	static float EaseInOutQuad(float t);
 };
