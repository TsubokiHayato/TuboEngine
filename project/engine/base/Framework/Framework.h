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
#endif // DEBUG
#include"SceneManager.h"
#include"ParticleCommon.h"
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

	//BlendModeの設定
	void SetBlendMode(int mode) { blendModeNum = mode; }
	int GetBlendMode() { return blendModeNum; }

protected:
	bool endRequest = false;

	//ブレンドモード
	int blendModeNum = 0;

	//基盤システム
	std::unique_ptr<WinApp> winApp = nullptr;
	//DirectX共通部分
	std::unique_ptr <DirectXCommon> dxCommon = nullptr;
	//スプライト共通部分
	std::unique_ptr <SpriteCommon> spriteCommon = nullptr;
	//オブジェクト3Dの共通部分
	std::unique_ptr <Object3dCommon> object3dCommon = nullptr;
	//モデル共通部分
	std::unique_ptr <ModelCommon> modelCommon = nullptr;
	std::unique_ptr <SrvManager> srvManager = nullptr;
#ifdef _DEBUG
std::unique_ptr<ImGuiManager> imGuiManager = nullptr;
#endif // _DEBUG

	
	std::unique_ptr<SceneManager> sceneManager = nullptr;

	//パーティクル共通部分
	std::unique_ptr <ParticleCommon> particleCommon = nullptr;
};

