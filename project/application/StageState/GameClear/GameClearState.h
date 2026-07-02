#pragma once
#include "StageState/IStageState.h"
class StageScene;
/// <summary>
/// ゲームクリア時のステージステート。クリア演出とシーン遷移を行う。
/// </summary>
class GameClearState : public IStageState {
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
};
