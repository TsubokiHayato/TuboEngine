#include<Framework.h>
#include<MyGame.h>

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {


	Framework* framework = new MyGame();
	framework->Run();

	delete framework;
	
	return 0;

}