#pragma once
#include "Camera.h"
#include "DebugCamera.h"
#include "IScene.h"
#include "Object3d.h"
#include "Particle.h"
#include "ParticleEmitter.h"
#include "SceneManager.h"
#include "UI/Title/TitleUI.h"
#include <random>
#include "Animation/SceneChangeAnimation.h"

#include <memory>
#include "Sprite.h"
#include "Character/Player/Player.h"
#include"application/SkyDome/SkyDome.h"

/// <summary>
/// タイトル画面のシーン。メニュー操作・デモ再生への移行・ステージシーンへの遷移を行う。
/// </summary>
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
	TuboEngine::Camera* GetMainCamera() const { return camera.get(); }

	/// <summary>
	/// 指定シーン番号へ遷移する。
	/// </summary>
	void ChangeNextScene(int sceneNo) { SceneManager::GetInstance()->ChangeScene(sceneNo); }

private:
	std::unique_ptr<TuboEngine::Camera> camera;
	TuboEngine::DebugCamera debugCamera_; // デバッグ用フリーカメラ（F2でON/OFF）
	TuboEngine::Math::Vector3 cameraPosition = {0.0f, 0.0f, -10.0f};
	TuboEngine::Math::Vector3 cameraRotation = {0.0f, 0.0f, 0.0f};
	TuboEngine::Math::Vector3 cameraScale = {1.0f, 1.0f, 1.0f};

	// --- ここを追加 ---
	std::unique_ptr<SceneChangeAnimation> sceneChangeAnimation;
	bool isRequestSceneChange = false;

	std::unique_ptr<TitleUI> titleUI;

	// Player (タイトル用アニメ)
	std::unique_ptr<Player> player_;
	bool playerIntroDone_ = false;
	float playerIntroTimer_ = 0.0f;
	float playerIntroDuration_ = 1.6f; // 端から来て止まる時間

	// 呼吸やHopなどのパラメータ
	float playerIdleTime_ = 0.0f;
	float time_ = 0.0f; // 背景アニメーション用タイマー

	// デモモード遷移用タイマー
	float demoTimer_ = 0.0f;
	const float kDemoStartTime = 10.0f;
	// アニメ経由での遷移先を保持する（UI以外からの要求、例: デモ）
	SceneType pendingNextSceneType_ = SceneType::Title;

	// タイトル画面用スカイドーム
	std::unique_ptr<SkyDome> skyDome_;
	float skyboxRotateY_ = 0.0f;
	float skyboxRotateX_ = 0.0f;
};
