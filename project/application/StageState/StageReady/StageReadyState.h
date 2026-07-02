#pragma once
#include "StageState/IStageState.h"
#include "Vector3.h"
#include <chrono>
#include <memory>
#include "Sprite.h"
class StageScene;

/// <summary>
/// ステージ開始前の準備状態（Ready）
/// ・配置は即座に行う
/// ・READY/START!! 表示だけ行ってから Playing に遷移する
/// </summary>
class StageReadyState : public IStageState {
public:
	/// <summary>
	/// ステートに入ったときの初期化処理。
	/// </summary>
	void Enter(StageScene* scene) override;
	/// <summary>
	/// 更新処理。
	/// </summary>
	void Update(StageScene* scene) override;
	/// <summary>
	/// ステートを抜けるときの後処理。
	/// </summary>
	void Exit(StageScene* scene) override;
	/// <summary>
	/// 3Dオブジェクト描画。
	/// </summary>
	void Object3DDraw(StageScene* scene) override;
	/// <summary>
	/// スプライト描画。
	/// </summary>
	void SpriteDraw(StageScene* scene) override;
	/// <summary>
	/// ImGui描画。
	/// </summary>
	void ImGuiDraw(StageScene* scene) override;
	/// <summary>
	/// パーティクル描画。
	/// </summary>
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

	std::unique_ptr<TuboEngine::Sprite> readySprite_;
	std::unique_ptr<TuboEngine::Sprite> startSprite_;
};
