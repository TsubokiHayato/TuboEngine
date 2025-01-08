#include "Framework.h"


void Framework::Initialize()
{

#pragma region 基盤システムの初期化

	//ウィンドウズアプリケーション

	winApp = new WinApp();
	winApp->Initialize();
#ifdef DEBUG
	//リークチェッカー
	D3DResourceLeakChecker leakChecker;
#endif // _DEBUG

	//DirectX共通部分

	dxCommon = new DirectXCommon();
	dxCommon->Initialize(winApp);

	//スプライト共通部分

	spriteCommon = new SpriteCommon;
	spriteCommon->Initialize(dxCommon);



	//オブジェクト3Dの共通部分

	object3dCommon = new Object3dCommon();
	object3dCommon->Initialize(dxCommon);

	//モデル共通部分

	modelCommon = new ModelCommon();
	modelCommon->Initialize(dxCommon);



	srvManager = new SrvManager();
	srvManager->Initialize(dxCommon);

#pragma endregion 基盤システムの初期化


#pragma region TextureManegerの初期化
	//テクスチャマネージャーの初期化
	TextureManager::GetInstance()->Initialize(dxCommon, srvManager);

	

#pragma endregion TextureManegerの初期化

#pragma region ModelManagerの初期化
	//モデルマネージャーの初期化
	ModelManager::GetInstance()->initialize(dxCommon);

	

#pragma endregion ModelManagerの初期化

#pragma region ImGuiManagerの初期化
#ifdef _DEBUG

	//ImGuiの初期化

	imGuiManager = std::make_unique<ImGuiManager>();
	imGuiManager->Initialize(winApp, dxCommon);

#endif // DEBUG

#pragma endregion ImGuiManagerの初期化

#pragma region AudioCommonの初期化
	//オーディオ共通部
	AudioCommon::GetInstance()->Initialize();
	

#pragma endregion AudioCommonの初期化


#pragma region Inputの初期化
	//入力初期化
	Input::GetInstance()->Initialize(winApp);
#pragma endregion Inputの初期化

}

void Framework::Update()
{
	//メッセージ処理
	if (winApp->ProcessMessage()) {
		endRequest = true;
	}
	Input::GetInstance()->Update();
	
}

void Framework::Finalize()
{

	//スプライト共通部分の削除
	delete spriteCommon;
	//WindowsAppの削除
		winApp->Finalize();
	delete winApp;
	winApp = nullptr;

	//DirectX共通部分の削除
	CloseHandle(dxCommon->GetFenceEvent());
	delete dxCommon;

	AudioCommon::GetInstance()->Finalize();

	Input::GetInstance()->Finalize();
	delete object3dCommon;
	delete modelCommon;
	delete srvManager;

#ifdef _DEBUG
	imGuiManager->Finalize();
#endif // DEBUG


}


void Framework::Run()
{
	Initialize();
	while (true)
	{
		Update();
		if (IsEndRequest())
		{
			break;
		}
		Draw();
	}
}
