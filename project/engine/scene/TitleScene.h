#pragma once
#include "Camera.h"
#include "IScene.h"
#include "Object3d.h"
#include "Particle.h"
#include "ParticleEmitter.h"
#include "SceneManager.h"
#include "TitleUI.h"
#include <random>
#include "Animation/SceneChangeAnimation.h"

#include <memory>
#include "Sprite.h"
#include "Character/Player/Player.h" // 追加: Player

class TitleScene : public IScene {
public:
	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize() override;

	/// <summary>
	/// 更新
	/// </summary>
	void Update() override;

	/// <summary>
	/// 終了処理
	/// </summary>
	void Finalize() override;

	/// <summary>
	/// 3Dオブジェクト描画
	/// </summary>
	void Object3DDraw() override;

	/// <summary>
	/// スプライト描画
	/// </summary>
	void SpriteDraw() override;

	/// <summary>
	/// ImGui描画
	/// </summary>
	void ImGuiDraw() override;

	/// <summary>
	/// パーティクル描画
	/// </summary>
	void ParticleDraw() override;

	/// <summary>
	/// カメラの取得
	/// </summary>
	Camera* GetMainCamera() const { return camera.get(); }

private:
	std::unique_ptr<Camera> camera;
	Vector3 cameraPosition = {0.0f, 0.0f, -10.0f};
	Vector3 cameraRotation = {0.0f, 0.0f, 0.0f};
	Vector3 cameraScale = {1.0f, 1.0f, 1.0f};

	// --- ここを追加 ---
	std::unique_ptr<SceneChangeAnimation> sceneChangeAnimation;
	bool isRequestSceneChange = false;

	std::unique_ptr<TitleUI> titleUI;

	// Player (タイトル用アニメ)
	std::unique_ptr<Player> player_;
	bool playerIntroDone_ = false;
	float playerIntroTimer_ = 0.0f;
	float playerIntroDuration_ = 1.6f; // 端から来て止まる時間

	// 呼吸やhopなどのパラメータ
	float playerIdleTime_ = 0.0f;
	float time_ = 0.0f; // 背景アニメーション用タイマー
};
