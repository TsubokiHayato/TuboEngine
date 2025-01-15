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
//#ifdef _DEBUG
//
//extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
//
//#endif // DEBUG
//



class DebugScene :public IScene
{
	//--------------------------------------
	//メンバ関数
public:
	void Initialize(Object3dCommon* object3dCommon,SpriteCommon* spriteCommon ,WinApp* winApp,DirectXCommon* dxCommon)override;
	void Update()override;
	void Finalize()override;
	void Object3DDraw()override;
	void SpriteDraw()override;
	void ImGuiDraw()override;
	//--------------------------------------
	//静的メンバ変数
private:
	
	WinApp* winApp = nullptr;
	DirectXCommon* dxCommon = nullptr;
	Object3dCommon* object3dCommon = nullptr;
	SpriteCommon* spriteCommon = nullptr;
	//--------------------------------------
	//メンバ変数
private:


	std::unique_ptr<Audio> audio = nullptr;

	bool endRequest = false;


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

	Object3d* object3d;
	Vector3 modelPosition = { -1.0f,0.0f,0.0f };
	Vector3 modelRotation = { 0.0f,0.0f,0.0f };
	Vector3 modelScale = { 1.0f,1.0f,1.0f };

	

	Object3d* object3d2;
	Vector3 modelPosition2 = { 1.0f,0.0f,0.0f };
	Vector3 modelRotation2 = { 0.0f,0.0f,0.0f };
	Vector3 modelScale2 = { 1.0f,1.0f,1.0f };

	


	Camera* camera = nullptr;
	Vector3 cameraPosition = { 0.0f,0.0f,-15.0f };
	Vector3 cameraRotation = { 0.0f,0.0f,0.0f };
	Vector3 cameraScale = { 1.0f,1.0f,1.0f };


};

