#include "Order.h"

void Order::Initialize()
{
	//初期化
	Framework::Initialize();
}

void Order::Update()
{
	//更新
	Framework::Update();

	
}

void Order::Finalize()
{
	//終了処理
	Framework::Finalize();

}

void Order::Draw()
{

	//RenderTargetの描画
	
	Framework::FrameWorkRenderTargetPreDraw();
	//3Dオブジェクト描画
	Framework::Object3dCommonDraw();
	//2Dスプライト描画
	Framework::SpriteCommonDraw();
	//パーティクル描画
	Framework::ParticleCommonDraw();
	Framework::FrameWorkRenderTargetPostDraw();

	//SwapChainの描画

	//ループ前処理
	Framework::FrameworkSwapChainPreDraw();
	//ImGuiの受付開始
	Framework::ImguiPreDraw();
	//ImGuiの受付終了
	Framework::ImguiPostDraw();
	//ループ後処理
	Framework::FrameworkSwapChainPostDraw();
}
