#pragma once
#include "StageState/IStageState.h"
#include <string>
#include <memory>
#include"Sprite.h"

// TutorialState:
// - 他Stateと合併しない「独立したState」
// - ReadyStateで用意したものを使わず、チュートリアル用CSVからこのState内で生成する
/// <summary>
/// チュートリアル用のステージステート。専用CSVからステージを生成して操作説明を行う。
/// </summary>
class TutorialState final : public IStageState {
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
	/// <summary>
	/// TutorialStage を構築する。
	/// </summary>
	void BuildTutorialStage(StageScene* scene);
	/// <summary>
	/// チュートリアル用スプライトの初期化。
	/// </summary>
	void InitializeTutorialSprites();

	float elapsed_ = 0.0f;
	int step_ = 0;
	bool built_ = false;
	std::string tutorialCsvPath_ = "Resources/Stage/Tutorial.csv";

	// Tutorial UI sprites (text baked into textures)
	std::unique_ptr<TuboEngine::Sprite> tutorialHeaderSprite_;
	std::unique_ptr<TuboEngine::Sprite> tutorialMoveSprite_;
	std::unique_ptr<TuboEngine::Sprite> tutorialAttackSprite_;
	std::unique_ptr<TuboEngine::Sprite> tutorialDashSprite_;
	std::unique_ptr<TuboEngine::Sprite> howtoNextSprite_;
	std::unique_ptr<TuboEngine::Sprite> titleBackSprite_;

	// Tutorial用: PlayerAutoController の有効フラグ（ImGui から ON/OFF）
	bool autoPlayEnabled_ = false;
};
