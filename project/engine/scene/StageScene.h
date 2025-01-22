#pragma once

#include"IScene.h"
class StageScene :public IScene
{
public:
	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="object3dCommon">3Dオブジェクト共通部分</param>
	/// <param name="spriteCommon">スプライト共通部分</param>
	/// <param name="particleCommon">パーティクル共通部分</param>
	/// <param name="winApp">ウィンドウアプリケーション</param>
	/// <param name="dxCommon">DirectX共通部分</param>
	void Initialize(Object3dCommon* object3dCommon, SpriteCommon* spriteCommon, ParticleCommon* particleCommon, WinApp* winApp, DirectXCommon* dxCommon);

	/// <summary>
	/// 更新
	/// </summary>
	void Update()override;

	/// <summary>
	/// 終了処理
	/// </summary>
	void Finalize()override;

	/// <summary>
	/// 3Dオブジェクト描画
	/// </summary>
	void Object3DDraw()override;

	/// <summary>
	/// スプライト描画
	/// </summary>
	void SpriteDraw()override;

	/// <summary>
	/// ImGui描画
	/// </summary>
	void ImGuiDraw()override;

	/// <summary>
	/// パーティクル描画
	/// </summary>
	void ParticleDraw()override;
};

