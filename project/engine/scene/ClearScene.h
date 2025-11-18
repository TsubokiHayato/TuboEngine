#pragma once
#include"IScene.h"
#include"LineManager.h"
#include"Object3d.h"
#include"Camera.h"
#include"Animation/SceneChangeAnimation.h"
#include"Sprite.h"
#include"Character/Player/Player.h"
#include <vector>
#include <memory>
#include <string>

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

	/// <summary>
	/// カメラの取得
	/// </summary>
	Camera* GetMainCamera() const {
		return camera.get(); }

private:

	std::unique_ptr<Camera> camera;           // カメラ
	Transform cameraTransform;                      // 変形情報

	// プレイヤー
	std::unique_ptr<Player> player_;

	// CLEAR 文字スプライト
	std::vector<std::unique_ptr<Sprite>> letterSprites_;
	std::vector<Vector2> letterBaseSizes_;
	std::vector<std::string> letterTextureNames_;

	// 表示・アニメパラメータ（ImGuiで操作可能)
	float elapsed_ = 0.0f;
	float letterDelay_ = 0.12f;
	float fadeDuration_ = 0.6f;
	float letterSpacing_ = 120.0f; // ピクセル間隔（ImGuiで変更可）
	float letterYOffset_ = 0.0f;   // 画面中心からの垂直オフセット

	// スペースで発動するプレイヤー用アニメーション（画面外へ飛ばす）
	bool spaceAnimActive_ = false;
	float spaceAnimTimer_ = 0.0f;
	float spaceAnimDuration_ = 1.2f; // 秒
	Vector3 spaceOrigPos_{};
	Vector3 spaceOrigRot_{};
	Vector3 spaceOrigScale_{};
	float spaceJumpHeight_ = 3.0f; // ジャンプ高さ(ワールド単位)
	bool spaceLaunchRight_ = true; // 発射方向を交互にするフラグ
	                               // シーン遷移管理フラグ
	bool isRequestSceneChange_ = false;
	std::unique_ptr<SceneChangeAnimation> sceneChangeAnimation_ = nullptr;

	 std::unique_ptr<Sprite> restartSprite_ = nullptr;
};

