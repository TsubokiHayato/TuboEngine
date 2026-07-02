#pragma once
#include "StageState/IStageState.h"
#include "Vector3.h"
#include <chrono>
#include <vector>
#include <memory>
#include "Sprite.h"


class StageScene;

/// Playing状態 （ステージプレイ中の状態）///
 class StagePlayingState : public IStageState {
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
	std::unique_ptr<TuboEngine::Sprite> pauseGuideSprite_ = nullptr; // ポーズ誘導UI

	// Playing用: PlayerAutoController の有効フラグ（ImGui から ON/OFF）
	bool autoPlayEnabled_ = false;
 };
