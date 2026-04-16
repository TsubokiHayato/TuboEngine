#pragma once
#include "IScene.h"
#include"SceneManager.h"
#include <memory>
#include <vector>
#include <string>
#include "Sprite.h"

#include "Vector2.h"
#include "Vector3.h"
#include "Transform.h"

class Camera;
class Player;
class SceneChangeAnimation;
class IParticleEmitter;

namespace TuboEngine {
    class Object3d;
    class TextObject;
}

class ClearScene : public IScene {
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

	void ChangeNextScene(int sceneNo) { SceneManager::GetInstance()->ChangeScene(sceneNo); }


private:
    // カメラ
	std::unique_ptr<TuboEngine::Camera> camera;
	TuboEngine::Transform cameraTransform{};

    // プレイヤー
    std::unique_ptr<Player> player_;

    // CLEAR 文字 (TextManager用)
    TuboEngine::TextObject* textClear_ = nullptr;
    float textFadeInTimer_ = 0.0f;

    // リスタート用テキスト
    TuboEngine::TextObject* textRestart_ = nullptr;
    float textRestartBlinkTimer_ = 0.0f;

    // お祝いパーティクル用エミッター
    IParticleEmitter* confettiGold_ = nullptr;
    IParticleEmitter* confettiColor_ = nullptr;
    bool hasEmittedConfetti_ = false;

    // 入場演出（シーンチェンジ終了後に開始）
    bool delayEntranceUntilSceneChangeDone_ = true; // かぶり防止のため既定ON
    bool entranceActive_ = false;               // 入場演出開始済み
    float entranceTimer_ = 0.0f;               // 入場演出の経過時間

    // 王冠（戴冠）演出
    std::unique_ptr<TuboEngine::Object3d> crownModel_;
    bool isCrowned_ = false;

    // シーンチェンジ
    std::unique_ptr<SceneChangeAnimation> sceneChangeAnimation_;
    bool isRequestSceneChange_ = false;

    // 経過時間
    float elapsed_ = 0.0f;

    // UI
    std::unique_ptr<TuboEngine::Sprite> restartSprite_;

private:

};

