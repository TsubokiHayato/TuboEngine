#pragma once
#include "IScene.h"
#include"SceneManager.h"
#include <memory>
#include <vector>
#include <string>

#include "Vector2.h"
#include "Vector3.h"
#include "Transform.h"

class Camera;
class Player;
class Sprite;
class SceneChangeAnimation;

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
	Camera* GetMainCamera() const { return camera.get(); }

	void ChangeNextScene(int sceneNo) { SceneManager::GetInstance()->ChangeScene(sceneNo); }


private:
    // カメラ
    std::unique_ptr<Camera> camera;
    Transform cameraTransform{};

    // プレイヤー
    std::unique_ptr<Player> player_;

    // 文字スプライト
    std::vector<std::unique_ptr<Sprite>> letterSprites_;
    std::vector<Vector2> letterBaseSizes_;
    std::vector<std::string> letterTextureNames_;
    float letterSpacing_ = 120.0f;
    float letterYOffset_ = 0.0f;
    float letterDelay_ = 0.06f;
    float fadeDuration_ = 0.6f;

    // 入場演出（シーンチェンジ終了後に開始）
    bool delayEntranceUntilSceneChangeDone_ = true; // かぶり防止のため既定ON
    bool lettersEnterActive_ = false;               // 入場演出開始済み
    float lettersEnterTimer_ = 0.0f;               // 入場演出の経過時間

    // クリア（退場）演出
    struct LetterClearAnim {
        Vector2 startPos{};
        Vector2 velocity{};
        float delay = 0.0f;
        float life = 0.6f;
        float startAlpha = 1.0f;
    };
    bool lettersClearActive_ = false;
    float lettersClearTimer_ = 0.0f;
    std::vector<LetterClearAnim> letterClearAnims_;

    // クリア演出パラメータ
    float lettersClearDuration_ = 0.7f;
    float lettersClearMaxDelay_ = 0.05f;
    float lettersClearOutwardSpeed_ = 280.0f;
    float lettersClearUpwardSpeed_ = 180.0f;
    float lettersClearEndScale_ = 0.55f;

    // シーンチェンジ
    std::unique_ptr<SceneChangeAnimation> sceneChangeAnimation_;
    bool isRequestSceneChange_ = false;

    // 経過時間
    float elapsed_ = 0.0f;

    // スペースジャンプ演出（既存）
    bool spaceAnimActive_ = false;
    float spaceAnimTimer_ = 0.0f;
    float spaceAnimDuration_ = 1.0f;
    float spaceJumpHeight_ = 0.5f;
    bool spaceLaunchRight_ = true;
    Vector3 spaceOrigPos_{};
    Vector3 spaceOrigRot_{};
    Vector3 spaceOrigScale_{};

    // UI
    std::unique_ptr<Sprite> restartSprite_;

private:
    void SetupLettersClearAnim(); // 文字のクリア演出初期化
};

