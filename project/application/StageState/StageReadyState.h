#pragma once
#include "IStageState.h"
#include "Vector3.h"
#include <chrono>
#include <vector>
#include <memory>
#include "Sprite.h"

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
	int currentDroppingLayer_ = 0;                   ///< 現在落下中のレイヤー
	bool isDropFinished_ = false;                    ///< アニメーション終了フラグ
	std::chrono::steady_clock::time_point prevTime_; ///< 前フレーム時刻

	// --- 各オブジェクトの落下目標座標 ---
	std::vector<Vector3> blockTargetPositions_;
	Vector3 playerTargetPosition_;
	std::vector<Vector3> enemyTargetPositions_;
	std::vector<Vector3> tileTargetPositions_;      // Tileの目標座標
	std::vector<Vector3> allDropTargetPositions_;

	// --- 各オブジェクトのレイヤー番号（チェビシェフ距離） ---
	std::vector<float> blockRippleLayers_;
	std::vector<float> enemyRippleLayers_;
	std::vector<float> tileRippleLayers_;           // Tileのレイヤー

	const float kDropDuration = 0.5f;
	const float kDropOffsetZ = 30.0f;
	
	float dropDuration_ = 0.5f; // 落下1レイヤーの所要時間
	float dropOffsetZ_ = 20.0f; // 落下開始時のZオフセット
	float dropTimer_ = 0.0f;

	float layerDropTimer_ = 0.0f; // 現在のレイヤーの落下下アニメーションタイマー

    enum class ReadyStatePhase {
        Ready,   // 落下アニメーション中
        Start,   // 落下完了直後
        None     // 何も表示しない
    };
    ReadyStatePhase readyPhase_ = ReadyStatePhase::Ready;
    float readyTimer_ = 0.0f;
    std::unique_ptr<Sprite> readySprite_;
    std::unique_ptr<Sprite> startSprite_;
	std::unique_ptr<Sprite> restartSprite_;
	

	// READY/START!!アニメ用
	float readyAppearAnim_ = 0.0f; // 0.0f～1.0f
	float startAppearAnim_ = 0.0f; // 0.0f～1.0f

	float restartWaitTimer_ = 0.0f;
};
