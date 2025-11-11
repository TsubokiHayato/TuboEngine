#pragma once
#include "Camera.h"
#include "IScene.h"
#include "LineManager.h"
#include "Object3d.h"
#include "Character/Player/Player.h"
#include "Sprite.h"
#include <vector>
#include <string>
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
	Camera* GetMainCamera() const { return camera.get(); }

private:
	std::unique_ptr<Camera> camera; // カメラ
	Transform cameraTransform;      // 変形情報

	std::unique_ptr<Player> player; // プレイヤー

	// 落下＋バウンス演出用（プレイヤー）
	float animTime_ = 0.0f;          // 経過時間
	float animDuration_ = 1.2f;      // 全体の時間（秒）
	Vector3 startPos_{-0.5f, 8.0f, 0.0f}; // 開始位置（Y>0）
	Vector3 endPos_{0.0f, 0.0f, 0.0f};    // 終了位置（Y=0）
	float xOffset_ = 0.8f;            // X方向に寄せる量（終点方向）
	bool loopBounce_ = false;         // ループ確認用

	// 横倒れ回転（Y軸回り）
	float tiltTargetRad_ = 1.57f;      // 目標角度（ラジアン）
	int tiltSign_ = 1;                // +1で右、-1で左

	// GAME OVER UI（各文字のドロップ演出）
	struct LetterAnim {
		std::unique_ptr<Sprite> sprite;
		Vector2 start;
		Vector2 end;
		float delay = 0.0f;
		float time = 0.0f;
	};
	std::vector<LetterAnim> letters_;
	float letterDuration_ = 0.6f;   // 各文字のドロップ時間
	float letterStagger_ = 0.08f;   // 文字ごとの遅延
	Vector2 letterSize_ = {96.0f, 96.0f};
	float lettersRowY_ = 80.0f;     // 到着Y
	float lettersStartY_ = -120.0f; // 開始Y（画面外上）
	float lettersGap_ = 6.0f;       // 文字間隔

	// 画像ファイルパスのプレフィックス（例: "Resources/UI/GameOver/")
	std::string letterTexturePrefix_ = "";
	// 互換のため、単一テクスチャ使用時のファイル名（未使用だが残置）
	std::string letterTexture_ = "barrier.png"; // 仮のテクスチャ

	
	std::unique_ptr<Sprite> restartSprite_;
};
