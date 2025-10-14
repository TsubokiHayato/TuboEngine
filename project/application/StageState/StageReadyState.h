#pragma once
#include "IStageState.h"
#include "Vector3.h"
#include <chrono>
#include <vector>

class StageScene;

/// <summary>
/// ステージ開始前の準備状態（Ready）
/// 各キャラクターの初期化やマップチップの読み込み、出現アニメーションを再生する
/// </summary>
class StageReadyState : public IStageState {
public:
	/// <summary>
	/// ステート開始時処理
	/// </summary>
	void Enter(StageScene* scene) override;

	/// <summary>
	/// 毎フレーム更新処理
	/// </summary>
	void Update(StageScene* scene) override;

	/// <summary>
	/// ステート終了時処理
	/// </summary>
	void Exit(StageScene* scene) override;

	/// <summary>
	/// 3Dオブジェクト描画
	/// </summary>
	void Object3DDraw(StageScene* scene) override;

	/// <summary>
	/// スプライト描画
	/// </summary>
	void SpriteDraw(StageScene* scene) override;

	/// <summary>
	/// ImGui描画
	/// </summary>
	void ImGuiDraw(StageScene* scene) override;

	/// <summary>
	/// パーティクル描画
	/// </summary>
	void ParticleDraw(StageScene* scene) override;

private:
	// --- アニメーション制御用 ---
	float dropTimer_ = 0.0f;                         ///< アニメーション全体タイマー
	int currentDroppingLayer_ = 0;                   ///< 現在落下中のレイヤー
	float layerDropTimer_ = 0.0f;                    ///< 現在レイヤーのタイマー
	bool isDropFinished_ = false;                    ///< アニメーション終了フラグ
	std::chrono::steady_clock::time_point prevTime_; ///< 前フレーム時刻

	// --- 各オブジェクトの落下目標座標 ---
	std::vector<Vector3> blockTargetPositions_;
	Vector3 playerTargetPosition_;
	std::vector<Vector3> enemyTargetPositions_;

	// --- 各オブジェクトのレイヤー番号（チェビシェフ距離） ---
	std::vector<float> blockRippleLayers_;
	std::vector<float> enemyRippleLayers_;

	const float kDropDuration = 0.5f;
	const float kDropOffsetZ = 30.0f;
};
