#include"engine/Framework/Framework.h"
#include"engine/Framework/Order.h"
#include"D3DResourceLeakChecker.h"
#include <memory>

/// <summary>
/// アプリケーションのエントリーポイント
/// </summary>
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {

#ifdef _DEBUG
	// リソースリークチェッカーのインスタンスを生成
	D3DResourceLeakChecker leakChecker;
#endif

	// フレームワークのインスタンスを生成（生 new/delete を使わない）
	auto framework = std::make_unique<Order>();

	// フレームワークを実行
	framework->Run();

	// 正常終了
	return 0;
}
