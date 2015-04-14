#include "systemclass.h"

SystemClass::SystemClass()
	: m_Input(0)
	, m_Graphics(0)
{}

SystemClass::SystemClass(const SystemClass& other)
{}

//cleanup in shutdown, since some windows functions do not call this function (ExitThread())
SystemClass::~SystemClass()
{}

bool SystemClass::Initialize()
{
	int screenWidth, screenHeight;
	screenWidth = 0;
	screenHeight = 0;

	//init windows API
	InitializeWindows(screenWidth, screenHeight);

	//create input object --> handles input from keyboard
	m_Input = new InputClass;
	if(!m_Input)
		return false;

	m_Input->Initialize();

	//create graphics object --> handles all rendering
	m_Graphics = new GraphicsClass;
	if(!m_Graphics)
		return false;

	bool result;
	result = m_Graphics->Initialize(screenWidth, screenHeight, m_hwnd);
	if(!result)
		return false;

	return true;
}

//clean up all variables
void SystemClass::Shutdown()
{
	//release graphics object
	if(!m_Graphics)
	{
		m_Graphics->Shutdown();
		delete m_Graphics;
		m_Graphics = 0;
	}

	//release input object
	if(!m_Input)
	{
		delete m_Input;
		m_Input = 0;
	}

	//shutdown the windows
	ShutdownWindows();

	return;
}

//checks if windows sended message and the application has to end
//otherwise it runs Frame()
void SystemClass::Run()
{
	//create and initalize message variable
	MSG msg;
	ZeroMemory(&msg, sizeof(MSG));

	//loop until quit from windows or user
	bool done, result;
	done = false;
	while(!done)
	{
		//handles Windows messages
		if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		//quit when Windows signals end of application
		if(msg.message == WM_QUIT)
			done = true;

		else
		{
			//process frame
			result = Frame();
			if(!result)
				done = true;
		}
	}

	return;
}

//actual code for application
bool SystemClass::Frame()
{
	//check if user pressed escape to end application
	if(m_Input->IsKeyDown(VK_ESCAPE))
		return false;

	//do frame processing for graphics object
	bool result;
	result = m_Graphics->Frame();
	if(!result)
		return false;

	return true;
}

//message handler - handle interesting information for us
//rest will be passed to Windows default message handler
LRESULT CALLBACK SystemClass::MessageHandler(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam)
{
	//check if key was pressed on keyboard
	if(umsg == WM_KEYDOWN)
	{
		//send to input object to record that state
		m_Input->KeyDown((unsigned int)wparam);
		return 0;
	}
	//check if key was released
	else if(umsg == WM_KEYUP)
	{
		//send to input object to unset the state for that key
		m_Input->KeyUp((unsigned int)wparam);
		return 0;
	}

	//send any other message to default message handler
	return DefWindowProc(hwnd, umsg, wparam, lparam);
}

//build window for application + returns screen width and height
//fullscreen can be set in graphics class via the constant FULL_SCREEN
void SystemClass::InitializeWindows(int& screenWidth, int& screenHeight)
{
	//get external pointer to this object
	ApplicationHandle = this;

	//get instance of application
	m_hinstance = GetModuleHandle(NULL);

	//set name for application
	m_applicationName = L"Engine";

	//setup windows class with default settings
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

	//register window class
	RegisterClassEx(&wc);

	//determine resolution of clients desktop screen
	screenWidth = GetSystemMetrics(SM_CXSCREEN);
	screenHeight = GetSystemMetrics(SM_CYSCREEN);

	//setup screen settings depending if it is running in full screen mode or not
	DEVMODE dmScreenSettings;
	int posX, posY = 0;
	if(FULL_SCREEN)
	{
		//set windows to maximum size and 32bit
		memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));
		dmScreenSettings.dmSize = sizeof(dmScreenSettings);
		dmScreenSettings.dmPelsWidth = (unsigned long)screenWidth;
		dmScreenSettings.dmPelsHeight = (unsigned long)screenHeight;
		dmScreenSettings.dmBitsPerPel = 32;
		dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

		//change display settings to full screen
		ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN);

		//set position to top left corner
		posX = posY = 0;
	}
	//set to 800x600 window
	else
	{
		screenWidth = 800;
		screenHeight = 600;

		//place window in middle of screen
		posX = (GetSystemMetrics(SM_CXSCREEN) - screenWidth) / 2;
		posY = (GetSystemMetrics(SM_CYSCREEN) - screenHeight) / 2;
	}

	//create window with screen setting and get handle to it
	m_hwnd = CreateWindowEx(WS_EX_APPWINDOW, m_applicationName, m_applicationName,
		WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_POPUP,
		posX, posY, screenWidth, screenHeight, NULL, NULL, m_hinstance, NULL);

	//bring windows on scrren and set main focus to it
	ShowWindow(m_hwnd, SW_SHOW);
	SetForegroundWindow(m_hwnd);
	SetFocus(m_hwnd);

	//hide mouse cursor
	ShowCursor(false);

	return;
}

//shut window down
//reset screen settings, releases window and associated handles
void SystemClass::ShutdownWindows()
{
	//show mouse cursor
	ShowCursor(true);

	//if leaving full screen mode, reset display settings
	if(FULL_SCREEN)
		ChangeDisplaySettings(NULL, 0);

	//remove window
	DestroyWindow(m_hwnd);
	m_hwnd = NULL;

	//remove application instance
	UnregisterClass(m_applicationName, m_hinstance);
	m_hinstance = NULL;

	//release pointer to this class
	ApplicationHandle = NULL;

	return;
}

//handle Windows message by passing them to our handler
LRESULT CALLBACK WndProc(HWND hwnd, UINT umessage, WPARAM wparam, LPARAM lparam)
{
	//message is destroyed or being closed
	if(umessage == WM_DESTROY || umessage == WM_CLOSE)
	{
		PostQuitMessage(0);
		return 0;
	}

	//all other messages are handled by message handler in system class
	return ApplicationHandle->MessageHandler(hwnd, umessage, wparam, lparam);
}