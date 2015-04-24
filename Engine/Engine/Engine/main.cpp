#include "systemclass.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pScmdline, int iCmdshow)
{
	//create new system object
	SystemClass* System = new SystemClass;
	if(!System)
		return 0;

	//init and run the system
	bool result = false;
	result = System->Initialize();
	if(result)
		System->Run();

	//shutdown and release system object after run ended
	System->Shutdown();
	delete System;
	System = 0;

	return 0;
}