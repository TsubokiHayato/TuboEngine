#include "Order.h"

void TuboEngine::Order::Initialize() {
	// 初期化
	TuboEngine::Framework::Initialize();
}

void TuboEngine::Order::Update() {
	// 更新
	TuboEngine::Framework::Update();
}

void TuboEngine::Order::Finalize() {
	// 終了処理
	TuboEngine::Framework::Finalize();
}

void TuboEngine::Order::Draw() {

	// RenderTargetの描画

	// Renderの設定
	TuboEngine::Framework::FrameWorkRenderTargetPreDraw();
	// 3Dオブジェクト描画
	TuboEngine::Framework::Object3dCommonDraw();
	// 2Dスプライト描画
	TuboEngine::Framework::SpriteCommonDraw();
	// ライン描画
	LineManager::GetInstance()->Draw();
	// パーティクル描画
	TuboEngine::Framework::ParticleCommonDraw();
	// swapChainのバリアを設定
	TuboEngine::Framework::FrameworkSwapChainPreDraw();
	// OffScreenの描画
	TuboEngine::Framework::OffScreenRenderingDraw();

	// SwapChainの描画

	// ループ前処理
#ifdef USE_IMGUI
	// ImGuiの受付開始
	Framework::ImguiPreDraw();
	// ImGuiの受付終了
	Framework::ImguiPostDraw();
#endif // USE_IMGUI

	// ループ後処理
	Framework::FrameworkSwapChainPostDraw();
}