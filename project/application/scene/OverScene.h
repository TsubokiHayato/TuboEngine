#pragma once
#include "Camera.h"
#include "DebugCamera.h"
#include "IScene.h"
#include "LineManager.h"
#include "Object3d.h"
#include "Character/Player/Player.h"
#include "Sprite.h"
#include <vector>
#include <string>

namespace TuboEngine { class TextObject; }

class OverScene : public IScene {
public:
	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize();

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

private:
	std::unique_ptr<TuboEngine::Camera> camera; // カメラ
	TuboEngine::Transform cameraTransform;      // 変形情報

	TuboEngine::DebugCamera debugCamera_;       // デバッグ用フリーカメラ（F2でON/OFF）

	std::unique_ptr<Player> player; // プレイヤー

	// 落下＋バウンス演出用（プレイヤー）
	float animTime_ = 0.0f;          // 経過時間
	float animDuration_ = 1.2f;      // 全体の時間（秒）
	TuboEngine::Math::Vector3 startPos_{-0.5f, 8.0f, 0.0f}; // 開始位置（Y>0）
	TuboEngine::Math::Vector3 endPos_{0.0f, 0.0f, 0.0f};    // 終了位置（Y=0）
	float xOffset_ = 0.8f;            // X方向に寄せる量（終点方向）
	bool loopBounce_ = false;         // ループ確認用

	// 横倒れ回転（Y軸回り）
	float tiltTargetRad_ = 1.57f;      // 目標角度（ラジアン）
	int tiltSign_ = 1;                // +1で右、-1で左

	// GAME OVER 文字列は TextManager で表示し、ここで演出する
	TuboEngine::TextObject* gameOverText_ = nullptr; // 演出対象（TextManager が所有）
	float goAnimTime_ = 0.0f;                         // 演出の経過時間
	TuboEngine::Math::Vector2 goStartPos_{640.0f, -120.0f}; // 落下開始（画面外上）
	TuboEngine::Math::Vector2 goEndPos_{640.0f, 200.0f};    // 着地位置
	float goDropDuration_ = 0.9f;  // 落下＋バウンドの時間
	float goStartScale_ = 3.0f;    // 落下開始時のスケール（インパクト用に大きく）
	float goEndScale_ = 1.0f;      // 着地後のスケール

};
