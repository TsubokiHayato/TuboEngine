#pragma once
#include"IScene.h"
#include"LineManager.h"
#include"Object3d.h"
#include "Animator.h"
#include"Camera.h"
class ClearScene :public IScene
{
public:
	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize();

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


	private:

	std::unique_ptr<Camera> camera;           // カメラ
	EulerTransform cameraTransform;                      // 変形情報

	EulerTransform transform_;               // 変形情報
	std::unique_ptr<Animator> animator; // アニメーションポインタ
};

