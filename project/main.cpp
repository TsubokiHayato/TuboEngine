#include<Framework.h>
#include"Order.h"

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {


	Framework* framework = new Order();

	framework->Run();

	delete framework;
	
	return 0;

}