#include "Framework.h"
#include"WinApp.h"

void Framework::Initialize()
{
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

	// リソースの有効性を確認
	if (!dxCommon->GetDevice() || !dxCommon->GetCommandList()) {
		throw std::runtime_error("DirectXリソースの初期化に失敗しました。");
	}

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

	//テクスチャマネージャーの初期化
	TextureManager::GetInstance()->Initialize(dxCommon.get(), srvManager.get());

	//モデルマネージャーの初期化
	ModelManager::GetInstance()->initialize(dxCommon.get());

	//オーディオ共通部
	AudioCommon::GetInstance()->Initialize();

	//入力初期化
	Input::GetInstance()->Initialize(winApp.get());


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

	//DirectX共通部分の終了
	dxCommon.reset();
	//WindowsAppの削除
	winApp->Finalize();
	winApp.reset();

}


void Framework::Run()
{
	//初期化
	Initialize();
	//メインループ
	while (true)
	{
		//更新
		Update();

		//終了リクエストがあったら
		if (IsEndRequest())
		{
			//ループを抜ける
			break;
		}

		//描画
		Draw();
	}
	//終了処理
	Finalize();
}

void Framework::FrameworkPreDraw()
{
	//描画前処理
	dxCommon->PreDraw();
	//ImGuiの受付開始
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
#ifdef _DEBUG
	//ImGuiの受付終了
	ImGui::ShowDemoWindow();
	//BlendMode変更
	ImGui::Begin("BlendNum");
	ImGui::Text("BlendMode");
	ImGui::Text("0: None");
	ImGui::Text("1: Normal");
	ImGui::Text("2: Add");
	ImGui::Text("3: Subtract");
	ImGui::Text("4: Multiply");
	ImGui::Text("5: Screen");
	ImGui::SliderInt("BlendNum", &objectBlendModeNum, 0, 5);
	ImGui::SliderInt("SpriteBlendNum", &spriteBlendModeNum, 0, 5);
	ImGui::End();

	

	imGuiManager->End();
#endif // _DEBUG

}

void Framework::Object3dCommonDraw()
{
	//オブジェクト3Dの描画
	object3dCommon->DrawSettingsCommon(objectBlendModeNum);
	sceneManager->Object3DDraw();
}

void Framework::SpriteCommonDraw()
{
	//スプライトの描画
	spriteCommon->DrawSettingsCommon(spriteBlendModeNum);
	sceneManager->SpriteDraw();
}

void Framework::ParticleCommonDraw()
{
	//パーティクルの描画
	particleCommon->DrawSettingsCommon();
	sceneManager->ParticleDraw();
}
