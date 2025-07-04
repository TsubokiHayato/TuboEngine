#include"Framework/Framework.h"
#include"Framework/Order.h"
#include"D3DResourceLeakChecker.h"

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {

#ifdef _DEBUG
    D3DResourceLeakChecker leakChecker; // ここで宣言
#endif

    //フレームワークのインスタンスを生成
    Framework* framework = new Order();

    //フレームワークを実行
    framework->Run();

    //フレームワークのインスタンスを破棄
    delete framework;

    // ここでleakCheckerのデストラクタが呼ばれる

    return 0;
}
