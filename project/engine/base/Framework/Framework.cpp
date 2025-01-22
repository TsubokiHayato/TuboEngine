#include "Framework.h"
#include"WinApp.h"

void Framework::Initialize()
{

#pragma region 基盤システムの初期化

	//ウィンドウズアプリケーション

	winApp = std::make_unique<WinApp>();
	winApp->Initialize();
#ifdef DEBUG
	//リークチェッカー
	D3DResourceLeakChecker leakChecker;
#endif // _DEBUG

	//DirectX共通部分

	dxCommon = std::make_unique<DirectXCommon>();
	dxCommon->Initialize(winApp.get());



#ifdef _DEBUG

	//ImGuiの初期化

	imGuiManager = std::make_unique<ImGuiManager>();
	imGuiManager->Initialize(winApp.get(), dxCommon.get());

#endif // DEBUG

	srvManager = std::make_unique<SrvManager>();
	srvManager->Initialize(dxCommon.get());

	//スプライト共通部分

	spriteCommon = std::make_unique<SpriteCommon>();
	spriteCommon->Initialize(winApp.get(), dxCommon.get());



	//オブジェクト3Dの共通部分

	object3dCommon = std::make_unique<Object3dCommon>();
	object3dCommon->Initialize(winApp.get(), dxCommon.get());

	//モデル共通部分

	modelCommon = std::make_unique<ModelCommon>();
	modelCommon->Initialize(dxCommon.get());

	//パーティクル共通部分
	particleCommon = std::make_unique<ParticleCommon>();
	particleCommon->Initialize(winApp.get(), dxCommon.get(), srvManager.get());



#pragma endregion 基盤システムの初期化


#pragma region TextureManegerの初期化
	//テクスチャマネージャーの初期化
	TextureManager::GetInstance()->Initialize(dxCommon.get(), srvManager.get());



#pragma endregion TextureManegerの初期化

#pragma region ModelManagerの初期化
	//モデルマネージャーの初期化
	ModelManager::GetInstance()->initialize(dxCommon.get());
#pragma endregion ModelManagerの初期化


#pragma endregion ImGuiManagerの初期化

#pragma region AudioCommonの初期化
	//オーディオ共通部
	AudioCommon::GetInstance()->Initialize();


#pragma endregion AudioCommonの初期化


#pragma region Inputの初期化
	//入力初期化
	Input::GetInstance()->Initialize(winApp.get());
#pragma endregion Inputの初期化


	//シーンマネージャーの初期化
	sceneManager = std::make_unique<SceneManager>();
	sceneManager->Initialize(object3dCommon.get(), spriteCommon.get(), particleCommon.get(), winApp.get(), dxCommon.get());

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
	AudioCommon::GetInstance()->Finalize();

	Input::GetInstance()->Finalize();

	//テクスチャマネージャの終了
	TextureManager::GetInstance()->Finalize();
	//モデルマネージャーの終了
	ModelManager::GetInstance()->Finalize();
	//DirectX共通部分の削除
	CloseHandle(dxCommon->GetFenceEvent());
	//WindowsAppの削除
	winApp->Finalize();

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
#ifdef _DEBUG
	//ImGuiの描画
	imGuiManager->Draw();
#endif // _DEBUG


	//描画
	dxCommon->PostDraw();
}

void Framework::ImguiPreDraw()
{
#ifdef _DEBUG
	//ImGuiの受付開始
	imGuiManager->Begin();

	sceneManager->ImGuiDraw();
#endif // _DEBUG

}

void Framework::ImguiPostDraw()
{
	//ImGuiの受付終了
	ImGui::ShowDemoWindow();
#ifdef _DEBUG

	imGuiManager->End();
#endif // _DEBUG


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
