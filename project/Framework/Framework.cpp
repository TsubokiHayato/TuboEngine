#include "Framework.h"
#include"WinApp.h"
#include"OffScreenRenderingPSO.h"
#include <dxcapi.h>



void Framework::Initialize() {
	
	WinApp::GetInstance()->Initialize();

#ifdef DEBUG
	//リークチェッカー
	D3DResourceLeakChecker leakChecker;
#endif // _DEBUG

	//DirectX共通部分

	
	DirectXCommon::GetInstance()->Initialize();

	// リソースの有効性を確認
	if (!DirectXCommon::GetInstance()->GetDevice() || !DirectXCommon::GetInstance()->GetCommandList()) {
		throw std::runtime_error("DirectXリソースの初期化に失敗しました。");
	}

#ifdef _DEBUG

	//ImGuiの初期化

	imGuiManager = std::make_unique<ImGuiManager>();
	imGuiManager->Initialize();

#endif // DEBUG

	srvManager = std::make_unique<SrvManager>();
	srvManager->Initialize();

	//スプライト共通部分

	spriteCommon = std::make_unique<SpriteCommon>();
	spriteCommon->Initialize();



	//オブジェクト3Dの共通部分
	object3dCommon = std::make_unique<Object3dCommon>();
	object3dCommon->Initialize();

	
	//パーティクル共通部分
	particleCommon = std::make_unique<ParticleCommon>();
	particleCommon->Initialize(srvManager.get());

	

	//テクスチャマネージャーの初期化
	TextureManager::GetInstance()->Initialize(srvManager.get());

	//モデルマネージャーの初期化
	ModelManager::GetInstance()->initialize();

	//オーディオ共通部
	AudioCommon::GetInstance()->Initialize();

	//入力初期化
	Input::GetInstance()->Initialize();

//オフスクリーンレンダリングの初期化
	offScreenRendering = std::make_unique<OffScreenRendering>();
	offScreenRendering->Initialize();

	//シーンマネージャーの初期化
	sceneManager = std::make_unique<SceneManager>();
	sceneManager->Initialize(object3dCommon.get(), spriteCommon.get(), particleCommon.get());


}
void Framework::Update() {
	//メッセージ処理
	if (WinApp::GetInstance()->ProcessMessage()) {
		endRequest = true;
	}
	//入力の更新
	Input::GetInstance()->Update();
	//シーンマネージャーの更新
	sceneManager->Update();
	//オフスクリーンレンダリングの更新
	offScreenRendering->Update();


}

void Framework::Finalize() {

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
	CloseHandle(DirectXCommon::GetInstance()->GetFenceEvent());

	
	//WindowsAppの削除
	WinApp::GetInstance()->Finalize();
	

}


void Framework::Run() {
	//初期化
	Initialize();
	//メインループ
	while (true) {
		//更新
		Update();

		//終了リクエストがあったら
		if (IsEndRequest()) {
			//ループを抜ける
			break;
		}

		//描画
		Draw();
	}
	//終了処理
	Finalize();
}

void Framework::FrameworkSwapChainPreDraw() {
	//描画前処理
	DirectXCommon::GetInstance()->PreDraw();

}

void Framework::FrameworkSwapChainPostDraw() {
#ifdef _DEBUG
	//ImGuiの描画
	imGuiManager->Draw();
#endif // _DEBUG

	offScreenRendering->TransitionRenderTextureToRenderTarget();
	//描画
	DirectXCommon::GetInstance()->PostDraw();
}

void Framework::ImguiPreDraw() {
#ifdef _DEBUG
	//ImGuiの受付開始
	imGuiManager->Begin();

	sceneManager->ImGuiDraw();

	offScreenRendering->DrawImGui();
#endif // _DEBUG

}

void Framework::ImguiPostDraw() {
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

void Framework::FrameWorkRenderTargetPreDraw() {

	//ImGuiの受付開始
	offScreenRendering->PreDraw();


	srvManager->PreDraw();

}




void Framework::Object3dCommonDraw() {
	//オブジェクト3Dの描画
	object3dCommon->DrawSettingsCommon(objectBlendModeNum);

	//3Dオブジェクトの描画
	sceneManager->Object3DDraw();


}

void Framework::SpriteCommonDraw() {
	//スプライトの描画
	spriteCommon->DrawSettingsCommon(spriteBlendModeNum);
	sceneManager->SpriteDraw();
}

void Framework::ParticleCommonDraw() {
	//パーティクルの描画
	particleCommon->DrawSettingsCommon();
	sceneManager->ParticleDraw();
}

void Framework::OffScreenRenderingDraw() {

	
	offScreenRendering->TransitionRenderTextureToShaderResource();

	//オフスクリーンレンダリングの描画
	offScreenRendering->Draw();
}

