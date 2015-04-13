#ifndef _SYSTEMCLASS_H_
#define _SYSTEMCLASS_H_


#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include "inputclass.h"
#include "graphicsclass.h"

class SystemClass
{
public:
	SystemClass();
	SystemClass(const SystemClass&);
	~SystemClass();

	bool Initialize();
	void Run();
	void Shutdown();

	LRESULT CALLBACK MessageHandler(HWND, UINT, WPARAM, LPARAM);

private:
	bool Frame();
	void InitializeWindows(int&, int&);
	void ShutdownWindows();

	//member variables
	LPCWSTR m_applicationName;
	HINSTANCE m_hinstance;
	HWND m_hwnd;

	InputClass* m_Input;
	GraphicsClass* m_Graphics;
};

//function prototypes
static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

//global pointer to class is necessary, to handle messages from Windows via the classes functions
static SystemClass* ApplicationHandle = 0;

#endif //_SYSTEMCLASS_H_