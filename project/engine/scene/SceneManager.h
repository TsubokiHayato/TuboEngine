#pragma once
#include<memory>
#include"IScene.h"
class SceneManager
{

public:
	//初期化
	void Initialize(Object3dCommon* object3dCommon, SpriteCommon* spriteCommon,ParticleCommon*particleCommon ,WinApp* winApp, DirectXCommon* dxCommon);
	//更新
	void Update();
	//終了処理
	void Finalize();
	//オブジェクト3D描画
	void Object3DDraw();
	//スプライト描画
	void SpriteDraw();
	//ImGui描画
	void ImGuiDraw();

	void ParticleDraw();

private:
	
	//オブジェクト3D共通部
	Object3dCommon* object3dCommon;
	//スプライト共通部
	SpriteCommon* spriteCommon;
	//パーティクル共通部
	ParticleCommon* particleCommon;
	//ウィンドウズアプリケーション
	WinApp* winApp;
	//DirectX共通部
	DirectXCommon* dxCommon;

private:

	//現在のシーン
	std::unique_ptr<IScene> currentScene = nullptr;
	//前のシーン
	std::unique_ptr<IScene> prevScene = nullptr;
	//現在のシーン番号
	int currentSceneNo = 0;
	//前のシーン番号
	int prevSceneNo = 0;
};

