#pragma once

#include "StageState/IStageState.h"
#include <memory>

#include "Sprite.h"

// PauseState:
// - Overlay pause menu shown during gameplay.
// - ESC toggles resume.
// - Provide simple options: Resume / Restart / Title.
/// <summary>
/// ポーズ中のステージステート。再開・リスタート・タイトルへの遷移を提供する。
/// </summary>
class PauseState final : public IStageState {
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
	/// メニューカーソルの更新。
	/// </summary>
	void UpdateCursor();

	static constexpr int kItemCount = 3;
	int selected_ = 0; // 0: Resume, 1: Restart, 2: Title
	std::unique_ptr<TuboEngine::Sprite> background_; 
	std::unique_ptr<TuboEngine::Sprite> blackout_;
	std::unique_ptr<TuboEngine::Sprite> cursor_;
	std::unique_ptr<TuboEngine::Sprite> resumeText_;
	std::unique_ptr<TuboEngine::Sprite> restartText_;
	std::unique_ptr<TuboEngine::Sprite> titleText_;
};
