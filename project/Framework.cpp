#include "Framework.h"
#include"WinApp.h"

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



#ifdef _DEBUG

	//ImGuiの初期化

	imGuiManager = std::make_unique<ImGuiManager>();
	imGuiManager->Initialize(winApp, dxCommon);

#endif // DEBUG

	srvManager = new SrvManager();
	srvManager->Initialize(dxCommon);

	//スプライト共通部分

	spriteCommon = new SpriteCommon;
	spriteCommon->Initialize(winApp,dxCommon);



	//オブジェクト3Dの共通部分

	object3dCommon = new Object3dCommon();
	object3dCommon->Initialize(winApp,dxCommon);

	//モデル共通部分

	modelCommon = new ModelCommon();
	modelCommon->Initialize(dxCommon);

	particleCommon = new ParticleCommon();
	particleCommon->Initialize(winApp, dxCommon,srvManager);

	

#pragma endregion 基盤システムの初期化


#pragma region TextureManegerの初期化
	//テクスチャマネージャーの初期化
	TextureManager::GetInstance()->Initialize(dxCommon, srvManager);

	

#pragma endregion TextureManegerの初期化

#pragma region ModelManagerの初期化
	//モデルマネージャーの初期化
	ModelManager::GetInstance()->initialize(dxCommon);

	

#pragma endregion ModelManagerの初期化


#pragma endregion ImGuiManagerの初期化

#pragma region AudioCommonの初期化
	//オーディオ共通部
	AudioCommon::GetInstance()->Initialize();
	

#pragma endregion AudioCommonの初期化


#pragma region Inputの初期化
	//入力初期化
	Input::GetInstance()->Initialize(winApp);
#pragma endregion Inputの初期化


	//シーンマネージャーの初期化
	sceneManager = std::make_unique<SceneManager>();
	sceneManager->Initialize(object3dCommon,spriteCommon,particleCommon,winApp,dxCommon);

}
void Framework::Update()
{
	//メッセージ処理
	if (winApp->ProcessMessage()) {
		endRequest = true;
	}
	//入力の更新
	Input::GetInstance()->Update();
	//シーンマネージャーの更新
	sceneManager->Update();
	
	
}

void Framework::Finalize()
{

	//ImGuiManagerの終了
#ifdef _DEBUG
	imGuiManager->Finalize();
#endif // DEBUG

	//スプライト共通部分の削除
	delete spriteCommon;
	


	AudioCommon::GetInstance()->Finalize();

	Input::GetInstance()->Finalize();

	//テクスチャマネージャの終了
	TextureManager::GetInstance()->Finalize();
	//モデルマネージャーの終了
	ModelManager::GetInstance()->Finalize();

	delete object3dCommon;
	delete particleCommon;
	delete modelCommon;
	delete srvManager;

	//DirectX共通部分の削除
	CloseHandle(dxCommon->GetFenceEvent());
	delete dxCommon;
//WindowsAppの削除
		winApp->Finalize();
	delete winApp;
	winApp = nullptr;

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
	Finalize();
}

void Framework::FrameworkPreDraw()
{
	dxCommon->PreDraw();
	srvManager->PreDraw();
}

void Framework::FrameworkPostDraw()
{
	//ImGuiの描画
	imGuiManager->Draw();
	//描画
	dxCommon->PostDraw();
}

void Framework::ImguiPreDraw()
{
	//ImGuiの受付開始
	imGuiManager->Begin();
#ifdef _DEBUG
	sceneManager->ImGuiDraw();
#endif // _DEBUG

}

void Framework::ImguiPostDraw()
{
	//ImGuiの受付終了
	ImGui::ShowDemoWindow();
	imGuiManager->End();

}

void Framework::Object3dCommonDraw()
{
	//オブジェクト3Dの描画
	object3dCommon->DrawSettingsCommon();
	sceneManager->Object3DDraw();
}

void Framework::SpriteCommonDraw()
{
	//スプライトの描画
	spriteCommon->DrawSettingsCommon();
	sceneManager->SpriteDraw();
}

void Framework::ParticleCommonDraw()
{
	particleCommon->DrawSettingsCommon();
	//パーティクルの描画
	sceneManager->ParticleDraw();
}
