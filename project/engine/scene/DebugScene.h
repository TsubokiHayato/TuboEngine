#pragma once
#include"WinApp.h"
#include"DirectXCommon.h"
#include"IScene.h"

#include"MT_Matrix.h"
#include "Input.h"
#include"Audio.h"
#include"Camera.h"
#include"Sprite.h"
#include"Object3d.h"
#include"Model.h"
#include <iostream>
#include <algorithm>

#include"SpriteCommon.h"
#include"Object3dCommon.h"
#include"ModelCommon.h"
#include"TextureManager.h"
#include"ModelManager.h"
#include"AudioCommon.h"
#include"ImGuiManager.h"


#undef min//minマクロを無効化
#undef max//maxマクロを


#pragma comment(lib,"dxguid.lib")//DirectXのライブラリ
#pragma comment(lib,"dxcompiler.lib")//DirectXのライブラリ


# define PI 3.14159265359f

class DebugScene :public IScene
{

public:
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
	
private:

	WinApp* winApp = nullptr;
	DirectXCommon* dxCommon = nullptr;
	Object3dCommon* object3dCommon = nullptr;
	SpriteCommon* spriteCommon = nullptr;

private:

	///Audio///
	std::unique_ptr<Audio> audio = nullptr;
	
	///Sprite///

	//左右反転フラグ
	bool isFlipX_;
	//上下反転フラグ
	bool isFlipY_;
	//テクスチャの左上座標
	Vector2 textureLeftTop;
	//テクスチャから初期サイズを得るフラグ
	bool isAdjustTextureSize;

	std::vector<Sprite*> sprites;

	///Model///

	std::unique_ptr<Object3d> object3d;
	Vector3 modelPosition = { 0.0f,0.0f,0.0f };
	Vector3 modelRotation = { 0.0f,0.0f,0.0f };
	Vector3 modelScale = { 1.0f,1.0f,1.0f };



	std::unique_ptr <Object3d> object3d2;
	Vector3 modelPosition2 = { 0.0f,0.0f,0.0f };
	Vector3 modelRotation2 = { 0.0f,0.0f,0.0f };
	Vector3 modelScale2 = { 1.0f,1.0f,1.0f };


	//Camera///

	std::unique_ptr <Camera> camera = nullptr;
	Vector3 cameraPosition = { 0.0f,0.0f,-15.0f };
	Vector3 cameraRotation = { 0.0f,0.0f,0.0f };
	Vector3 cameraScale = { 1.0f,1.0f,1.0f };

	int lightType = 0;
	
	Vector3 lightDirection = { 0.0f,-1.0f,0.0f };
	Vector4 lightColor = { 1.0f,1.0f,1.0f,1.0f };
	float intensity = 1.0f;
	float shininess = 1.0f;

	Vector3 pointLightPosition = { 0.0f,0.0f,0.0f };
	Vector4 pointLightColor = { 1.0f,1.0f,1.0f,1.0f };
	float pointLightIntensity = 1.0f;

	
};

