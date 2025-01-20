#pragma once

#include"DirectXcommon.h"
#include"D3DResourceLeakChecker.h"
#include"SpriteCommon.h"
#include"Object3dCommon.h"
#include"ModelCommon.h"
#include"TextureManager.h"
#include"ModelManager.h"
#include <SrvManager.h>
#include"AudioCommon.h"
#ifdef _DEBUG
#include"ImGuiManager.h"
#include"SceneManager.h"
#include"ParticleCommon.h"
#endif // DEBUG

#include"Input.h"

class Framework
{
public:

	virtual ~Framework() = default;

	//初期化
	virtual void Initialize();

	//更新処理
	virtual void Update();

	//終了処理
	virtual void Finalize();

	//描画
	virtual void Draw() = 0;

public:

	void FrameworkPreDraw();
	void FrameworkPostDraw();
	void ImguiPreDraw();
	void ImguiPostDraw();
	void Object3dCommonDraw();
	void SpriteCommonDraw();
	void ParticleCommonDraw();

public:
	
		//終了リクエストがあったかどうか
		virtual bool IsEndRequest() { return endRequest; }

public:
	void Run();

protected:
	bool endRequest = false;

	//基盤システム
	WinApp* winApp = nullptr;
	//DirectX共通部分
	DirectXCommon* dxCommon = nullptr;
	//スプライト共通部分
	SpriteCommon* spriteCommon = nullptr;
	//オブジェクト3Dの共通部分
	Object3dCommon* object3dCommon = nullptr;
	//モデル共通部分
	ModelCommon* modelCommon = nullptr;
	SrvManager* srvManager = nullptr;
	std::unique_ptr<ImGuiManager> imGuiManager = nullptr;
	std::unique_ptr<SceneManager> sceneManager = nullptr;

	//パーティクル共通部分
	ParticleCommon* particleCommon = nullptr;
};

