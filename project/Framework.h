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

	//終了リクエストがあったかどうか
	virtual bool IsEndRequest() { return endRequest; }

public:
	void Run();

protected:
	bool endRequest = false;

	WinApp* winApp = nullptr;
	DirectXCommon* dxCommon = nullptr;
	SpriteCommon* spriteCommon = nullptr;
	Object3dCommon* object3dCommon = nullptr;
	ModelCommon* modelCommon = nullptr;
	SrvManager* srvManager = nullptr;
	std::unique_ptr<ImGuiManager> imGuiManager = nullptr;

};

