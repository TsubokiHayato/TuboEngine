#include"Framework/Framework.h"
#include"Framework/Order.h"

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {

	//フレームワークのインスタンスを生成
	Framework* framework = new Order();

	//フレームワークを実行
	framework->Run();

	//フレームワークのインスタンスを破棄
	delete framework;
	
	return 0;

}
