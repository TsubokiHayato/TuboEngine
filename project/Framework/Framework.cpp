#include "Framework.h"
#include"WinApp.h"
#include <dxcapi.h>



void Framework::Initialize() {

	WinApp::GetInstance()->Initialize();

	//DirectX共通部分


	DirectXCommon::GetInstance()->Initialize();

	// リソースの有効性を確認
	if (!DirectXCommon::GetInstance()->GetDevice() || !DirectXCommon::GetInstance()->GetCommandList()) {
		throw std::runtime_error("DirectXリソースの初期化に失敗しました。");
	}

#ifdef USE_IMGUI

	//ImGuiの初期化


	ImGuiManager::GetInstance()->Initialize();

#endif // USE_IMGUI

	//SRVマネージャーの初期化
	SrvManager::GetInstance()->Initialize();
	//スプライト共通部分
	SpriteCommon::GetInstance()->Initialize();

	//オブジェクト3Dの共通部分
	Object3dCommon::GetInstance()->Initialize();


	//パーティクル共通部分
	ParticleCommon::GetInstance()->Initialize();



	//テクスチャマネージャーの初期化
	TextureManager::GetInstance()->Initialize();

	//モデルマネージャーの初期化
	ModelManager::GetInstance()->initialize();

	//オーディオ共通部
	AudioCommon::GetInstance()->Initialize();

	//入力初期化
	Input::GetInstance()->Initialize(WinApp::GetInstance()->GetHWND());

	//オフスクリーンレンダリングの初期化
	OffScreenRendering::GetInstance()->Initialize();

	//ラインマネージャーの初期化
	LineManager::GetInstance()->Initialize();

	//シーンマネージャーの初期化
	SceneManager::GetInstance()->Initialize(STAGE); // タイトルシーンから開始

}
void Framework::Update() {
	//メッセージ処理
	if (WinApp::GetInstance()->ProcessMessage()) {
		endRequest = true;
	}
	//入力の更新
	Input::GetInstance()->Update();
	
	//シーンマネージャーの更新
	SceneManager::GetInstance()->Update();

	Camera* mainCamera = SceneManager::GetInstance()->GetMainCamera();

	if (mainCamera) {
		// ラインマネージャーのカメラ設定
		LineManager::GetInstance()->SetDefaultCamera(mainCamera);
		OffScreenRendering::GetInstance()->SetCamera(mainCamera);
	}

	//オフスクリーンレンダリングの更新
	OffScreenRendering::GetInstance()->Update();
//
	LineManager::GetInstance()->Update();
}
void Framework::Finalize() {
#ifdef USE_IMGUI
	ImGuiManager::GetInstance()->Finalize();
#endif // USE_IMGUI

	Input::GetInstance()->Finalize();

	// パーティクルマネージャの明示解放（エミッター内のGPUリソースを先に解放する）
	ParticleManager::GetInstance()->Finalize();

	TextureManager::GetInstance()->Finalize();
	ModelManager::GetInstance()->Finalize();
	CloseHandle(DirectXCommon::GetInstance()->GetFenceEvent());

	OffScreenRendering::GetInstance()->Finalize();
	SrvManager::GetInstance()->Finalize();
	SceneManager::GetInstance()->Finalize();

	ParticleCommon::GetInstance()->Finalize();
	SpriteCommon::GetInstance()->Finalize();
	Object3dCommon::GetInstance()->Finalize();
	SkyBoxCommon::GetInstance()->Finalize();
	LineManager::GetInstance()->Finalize();
	
	AudioCommon::GetInstance()->Finalize();
	DirectXCommon::GetInstance()->Finalize();
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
#ifdef USE_IMGUI
	//ImGuiの描画
	ImGuiManager::GetInstance()->Draw();
#endif // USE_IMGUI

	OffScreenRendering::GetInstance()->TransitionRenderTextureToRenderTarget();
	//描画
	DirectXCommon::GetInstance()->PostDraw();
}

void Framework::ImguiPreDraw() {
#ifdef USE_IMGUI
	//ImGuiの受付開始
	ImGuiManager::GetInstance()->Begin();

	SceneManager::GetInstance()->ImGuiDraw();

	OffScreenRendering::GetInstance()->DrawImGui();
#endif // USE_IMGUI

}

void Framework::ImguiPostDraw() {
#ifdef USE_IMGUI
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



	ImGuiManager::GetInstance()->End();
#endif // USE_IMGUI

}

void Framework::FrameWorkRenderTargetPreDraw() {

	//ImGuiの受付開始
	OffScreenRendering::GetInstance()->PreDraw();

	SrvManager::GetInstance()->PreDraw();

	
	
}




void Framework::Object3dCommonDraw() {
	//オブジェクト3Dの描画
	Object3dCommon::GetInstance()->DrawSettingsCommon(objectBlendModeNum);
	
	//3Dオブジェクトの描画
	SceneManager::GetInstance()->Object3DDraw();
	

}

void Framework::SpriteCommonDraw() {
	
	//スプライトの描画
	SpriteCommon::GetInstance()->DrawSettingsCommon(spriteBlendModeNum);
	SceneManager::GetInstance()->SpriteDraw();
	

}

void Framework::ParticleCommonDraw() {
	//パーティクルの描画
	ParticleCommon::GetInstance()->DrawSettingsCommon();
	SceneManager::GetInstance()->ParticleDraw();
}

void Framework::OffScreenRenderingDraw() {
	OffScreenRendering::GetInstance()->TransitionRenderTextureToShaderResource();

	OffScreenRendering::GetInstance()->TransitionDepthTo(D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

	//オフスクリーンレンダリングの描画
	OffScreenRendering::GetInstance()->Draw();
}

