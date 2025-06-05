#pragma once
#include "IScene.h"

#include "Input.h"
#include"Audio.h"
#include"Camera.h"


#include"SpriteCommon.h"
#include"Object3dCommon.h"
#include"ModelCommon.h"
#include"TextureManager.h"
#include"ModelManager.h"
#include"AudioCommon.h"
#include"ImGuiManager.h"


#include "Player.h"
#include "Enemy.h"



class StageScene : public IScene {
public:
	///---------------------------------------------------------------------------------------
	///				メンバ関数
	///----------------------------------------------------------------------------------------

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

	
private:

	///----------------------------------------------------------------------------------------
	///				引き渡し用変数
	///-----------------------------------------------------------------------------------------

	WinApp* winApp = nullptr;
	DirectXCommon* dxCommon = nullptr;
	Object3dCommon* object3dCommon = nullptr;
	SpriteCommon* spriteCommon = nullptr;

private:
	///----------------------------------------------------------------------------------------
	///				メンバ変数
	///----------------------------------------------------------------------------------------

	/// Audio///
	
	std::unique_ptr<Audio> audio = nullptr;

	/// Camera ///
	std::unique_ptr<Camera> camera = nullptr;
	Vector3 cameraPosition = {0.0f, 0.0f, -5.0f};
	Vector3 cameraRotation = {0.0f, 0.0f, 0.0f};
	Vector3 cameraScale = {1.0f, 1.0f, 1.0f};
	
	/// Player ///
	std::unique_ptr<Player> player = nullptr;
	/// Enemy ///
	std::unique_ptr<Enemy> enemy = nullptr;
	std::vector<std::unique_ptr<Enemy>> enemies; // Enemyリスト


};
