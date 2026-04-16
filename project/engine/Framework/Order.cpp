#include "Order.h"

void Order::Initialize() {
	//初期化
	Framework::Initialize();
}

void Order::Update() {
	//更新
	Framework::Update();


}

void Order::Finalize() {
	//終了処理
	Framework::Finalize();

}

void Order::Draw() {

	//RenderTargetの描画
	
	// Renderの設定
	Framework::FrameWorkRenderTargetPreDraw();
	//3Dオブジェクト描画
	Framework::Object3dCommonDraw();
	//2Dスプライト描画
	Framework::SpriteCommonDraw();
	// ライン描画
	LineManager::GetInstance()->Draw();
	//パーティクル描画
	Framework::ParticleCommonDraw();
	// swapChainのバリアを設定
	Framework::FrameworkSwapChainPreDraw();
	// OffScreenの描画
	Framework::OffScreenRenderingDraw();
	


	//SwapChainの描画

	//ループ前処理
#ifdef USE_IMGUI
//ImGuiの受付開始
	Framework::ImguiPreDraw();
	//ImGuiの受付終了
	Framework::ImguiPostDraw();
#endif // USE_IMGUI

	
	//ループ後処理
	Framework::FrameworkSwapChainPostDraw();
}
