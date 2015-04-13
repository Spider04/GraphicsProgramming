#include "systemclass.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pScmdline, int Cmdshow)
{
	SystemClass* system;
	bool result;

	//create new instance of SystemClass
	system = new SystemClass();
	if(!system)
		return 0;

	//initialize and run system
	result = system->Initialize();
	if(result)
		system->Run();

	//shutdown and delete it afterwards
	system->Shutdown();
	delete system;
	system = 0;

	return 0;
}