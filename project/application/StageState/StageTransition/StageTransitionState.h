#pragma once
#include "StageState/IStageState.h"
#include "Vector3.h"
#include <chrono>

/// <summary>
/// 次のステージへ移動する間のステージステート。遷移演出を行う。
/// </summary>
class StageTransitionState : public IStageState {
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
 	std::chrono::steady_clock::time_point startTime_{};
 	float durationSec_ = 1.5f; // イージング移動時間
 	float jumpHeight_ = 8.0f;  // ジャンプの最大高さ（壁を飛び越える）
 	TuboEngine::Math::Vector3 startPos_{};
 	TuboEngine::Math::Vector3 targetPos_{};
 	bool initialized_ = false;

 	/// <summary>
 	/// 2次のS字イージング（0..1 → 0..1）。
 	/// </summary>
 	static float EaseInOutQuad(float t);
 };
