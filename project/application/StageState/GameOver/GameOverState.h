#pragma once
class StageScene; 
#include "IStageState.h"
#include <memory>
#include "Animation/SceneChangeAnimation.h"

class Player;

class GameOverState : public IStageState
{
public:
    void Enter(StageScene* scene) override;
    void Update(StageScene* scene) override;
    void Exit(StageScene* scene) override;
	void Object3DDraw(StageScene* scene) override;
	void SpriteDraw(StageScene* scene) override;
	void ImGuiDraw(StageScene* scene) override;
	void ParticleDraw(StageScene* scene) override;

private:
	// アニメーション段階（カメラ移動→目が回る→転倒→暗転）
	enum class Phase { None,CameraMove, Dizzy,  Blackout, Done };
	Phase phase_ = Phase::CameraMove;

	// 対象
	Player* player_ = nullptr;

	// プレイヤー演出パラメータ（共通）
	float spinSpeed_ = 2.5f;      // 回転速度(rad/sec)
	float shrinkSpeed_ = 0.6f;    // 縮小率/秒（線形）
	float fadeSpeed_ = 1.2f;      // アルファ減少/秒
	float currentAlpha_ = 1.0f;
	float minScale_ = 0.1f;

	// クラクラ（揺れ）と転倒（傾き）
	float wobbleAmp_ = 0.4f;      // 初期揺れ振幅（rad）
	float wobbleDecay_ = 0.5f;    // 揺れの減衰/秒
	float wobbleFreq_ = 6.0f;     // 揺れの周波数（Hz）
	float tiltTarget_ = 1.3f;     // 目標の傾き角（rad）
	float tiltSpeed_ = 1.2f;      // 傾き速度（rad/sec）
	float fallMoveSpeed_ = 0.5f;  // 倒れる方向への移動速度
	Vector3 fallDir_ = {0.0f, 0.0f, 0.0f};

	// 目が回る（Dizzy）演出
	float dizzyDuration_ = 3.6f;  // 目が回る演出の長さ
	float dizzyTime_ = 0.0f;      // 経過時間
	float orbitSpeed_ = 8.0f;     // 図形の回転速度(rad/sec)
	float orbitRadius_ = 1.5f;    // 軌道半径
	float orbitHeightZ_ = 0.8f;  // 見やすいように手前に
	std::unique_ptr<class Object3d> dizzy1_;
	std::unique_ptr<class Object3d> dizzy2_;
	float dizzyAlpha_ = 1.0f;     // フェードアウト用

	// GameOver時の正面カメラ設定（リクエストに合わせて狙い値へイージング）
	float initialYaw_ = 0.0f;     // GameOver開始時のプレイヤー向き（Z軸）
	Vector3 camStartOffset_ = {0.0f, 0.0f, 0.0f};
	Vector3 camTargetOffset_ = {0.0f, 15.0f, 5.0f};
	Vector3 camStartRot_ = {0.0f, 0.0f, 0.0f};
	Vector3 camTargetRot_ = {1.9f, 0.0f, 0.0f};
	float camAnimTime_ = 0.0f;
	float camAnimDuration_ = 0.8f; // イージング時間

	// 進行
	float elapsed_ = 0.0f;
	
	// 暗転スプライト
	std::unique_ptr<class Sprite> blackoutSprite_;

	std::unique_ptr<Sprite> restartSprite_;
};