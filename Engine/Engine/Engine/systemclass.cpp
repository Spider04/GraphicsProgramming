#include "systemclass.h"

SystemClass::SystemClass()
	: m_Application(0)
{}
SystemClass::SystemClass(const SystemClass& other)
{
}

SystemClass::~SystemClass()
{
}


bool SystemClass::Initialize()
{
	//init windows API
	int screenWidth = 0, screenHeight = 0;
	InitializeWindows(screenWidth, screenHeight);

	//create and init application object
	m_Application = new ApplicationClass;
	if(!m_Application)
		return false;

	bool result = false;
	result = m_Application->Initialize(m_hinstance, m_hwnd, screenWidth, screenHeight);
	if(!result)
		return false;

	return true;
}

void SystemClass::InitializeWindows(int& screenWidth, int& screenHeight)
{
	//set external ppointer to this object
	ApplicationHandle = this;

	//get instance of this app
	m_hinstance = GetModuleHandle(NULL);

	//app name
	m_applicationName = L"Dungeon Crawler";

	//set up window with default settings
	WNDCLASSEX wc;
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = m_hinstance;
	wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);
	wc.hIconSm = wc.hIcon;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = m_applicationName;
	wc.cbSize = sizeof(WNDCLASSEX);

	//register window
	RegisterClassEx(&wc);

	//get screen resolution
	screenWidth = GetSystemMetrics(SM_CXSCREEN);
	screenHeight = GetSystemMetrics(SM_CYSCREEN);

	
	DEVMODE dmScreenSettings;
	int posX = 0, posY = 0;

	//check if full screen mode or not
	if (FULL_SCREEN)
	{
		//set screen to max size and 32bit
		memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));
		dmScreenSettings.dmSize = sizeof(dmScreenSettings);
		dmScreenSettings.dmPelsWidth = (unsigned long)screenWidth;
		dmScreenSettings.dmPelsHeight = (unsigned long)screenHeight;
		dmScreenSettings.dmBitsPerPel = 32;
		dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

		//change display settings to full screen
		ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN);

		//set window in top left corner;
		posX = posY = 0;
	}
	else
	{
		//set screen size to 800x600
		screenWidth = 800;
		screenHeight = 600;

		//place window in center of screen
		posX = (GetSystemMetrics(SM_CXSCREEN) - screenWidth) / 2;
		posY = (GetSystemMetrics(SM_CYSCREEN) - screenHeight) / 2;
	}

	//create window and get handler
	m_hwnd = CreateWindowEx(WS_EX_APPWINDOW, m_applicationName, m_applicationName,
		WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_POPUP,
		posX, posY, screenWidth, screenHeight, NULL, NULL, m_hinstance, NULL);

	//bring window in main focus up
	ShowWindow(m_hwnd, SW_SHOW);
	SetForegroundWindow(m_hwnd);
	SetFocus(m_hwnd);

	//do not show the mouse cursor
	ShowCursor(false);

	return;
}


void SystemClass::Shutdown()
{
	//release app object
	if(m_Application)
	{
		m_Application->Shutdown();
		delete m_Application;
		m_Application = 0;
	}

	//shutdown the window
	ShutdownWindows();
	
	return;
}

void SystemClass::ShutdownWindows()
{
	//show cursor again
	ShowCursor(true);

	//change display settings if we were in full screen mode
	if (FULL_SCREEN)
	{
		ChangeDisplaySettings(NULL, 0);
	}

	//remove window
	DestroyWindow(m_hwnd);
	m_hwnd = NULL;

	//remove app instance
	UnregisterClass(m_applicationName, m_hinstance);
	m_hinstance = NULL;

	//remove external pointer
	ApplicationHandle = NULL;

	return;
}


void SystemClass::Run()
{
	//init message structure
	MSG msg;
	ZeroMemory(&msg, sizeof(MSG));
	
	//loop until quit signal (from window or user)
	bool done = false;
	bool result = false;

	while(!done)
	{
		//handle window messages
		if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		//quit if the window says so
		if(msg.message == WM_QUIT)
			done = true;

		else
		{
			//process the frame
			result = Frame();
			if(!result)
				done = true;
		}
	}

	return;
}

bool SystemClass::Frame()
{
	//do frame processing from app
	bool result = false;
	result = m_Application->Frame();

	return result;
}


LRESULT CALLBACK SystemClass::MessageHandler(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam)
{
	return DefWindowProc(hwnd, umsg, wparam, lparam);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT umessage, WPARAM wparam, LPARAM lparam)
{
	switch(umessage)
	{
		//check if window got destroyed
		case WM_DESTROY:
		{
			PostQuitMessage(0);
			return 0;
		}

		//or is being closed
		case WM_CLOSE:
		{
			PostQuitMessage(0);		
			return 0;
		}

		//the rest of the messages will be handled by the default window handler
		default:
		{
			return ApplicationHandle->MessageHandler(hwnd, umessage, wparam, lparam);
		}
	}
}