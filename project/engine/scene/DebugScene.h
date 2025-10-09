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
#include"TextureManager.h"
#include"ModelManager.h"
#include"AudioCommon.h"
#include"ImGuiManager.h"

#include"SkyBox.h"
#include"SceneChangeAnimation.h"


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
	void Initialize();

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

	/// <summary>
	/// Camera取得
	/// </summary>
	Camera* GetMainCamera() const { return camera.get(); }

private:

	///Audio///
	std::unique_ptr<Audio> audio = nullptr;
	
	
	//Camera///

	std::unique_ptr <Camera> camera = nullptr;
	Vector3 cameraPosition = { 0.0f,1.0f,-15.0f };
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



	///SkyBox
	
	std::unique_ptr<SkyBox> skyBox = nullptr;

	/// SceneChangeAnimation
	std::unique_ptr<SceneChangeAnimation> sceneChangeAnimation = nullptr;
	bool isRequestSceneChange = false;

	/// GuideUISprite ///
	std::unique_ptr<Sprite> guideUISprite = nullptr;

};

