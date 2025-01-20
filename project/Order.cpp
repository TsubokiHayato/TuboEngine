#include "Order.h"

void Order::Initialize()
{
	Framework::Initialize();
}

void Order::Update()
{
	Framework::Update();

	Framework::ImguiPreDraw();

	Framework::ImguiPostDraw();
}

void Order::Finalize()
{
	Framework::Finalize();

}

void Order::Draw()
{
	//ループ前処理
	Framework::FrameworkPreDraw();
	//3Dオブジェクト描画
	Framework::Object3dCommonDraw();
	//2Dスプライト描画
	Framework::SpriteCommonDraw();
	//パーティクル描画
	Framework::ParticleCommonDraw();
	//ループ後処理
	Framework::FrameworkPostDraw();
}
