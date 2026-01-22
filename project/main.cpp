#include"engine/Framework/Framework.h"
#include"engine/Framework/Order.h"
#include"D3DResourceLeakChecker.h"

/// <summary>
/// アプリケーションのエントリーポイント
/// </summary>
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {

#ifdef _DEBUG
	// リソースリークチェッカーのインスタンスを生成
    D3DResourceLeakChecker leakChecker;
#endif

    //フレームワークのインスタンスを生成
    Framework* framework = new Order();

    //フレームワークを実行
    framework->Run();

    //フレームワークのインスタンスを破棄
    delete framework;

    // 正常終了
    return 0;
}
