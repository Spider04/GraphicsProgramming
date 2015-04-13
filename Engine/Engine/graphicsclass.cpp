#include "graphicsclass.h"

GraphicsClass::GraphicsClass()
	: m_D3D(0)
{}

GraphicsClass::GraphicsClass(const GraphicsClass& other)
{}

GraphicsClass::~GraphicsClass()
{}

//init directx system
bool GraphicsClass::Initialize(int screenWidth, int screenHeight, HWND hwnd)
{
	//create new Direct3D object
	m_D3D = new D3DClass;
	if(!m_D3D)
		return false;

	bool result;
	result = m_D3D->Initialize(screenWidth, screenHeight, VSYNC_ENABLED, hwnd, FULL_SCREEN, SCREEN_DEPTH, SCREEN_NEAR);
	if(!result)
	{
		MessageBox(hwnd, L"Could not initialize Direct3D", L"Error", MB_OK);
		return false;
	}

	return true;
}

void GraphicsClass::Shutdown()
{
	if(m_D3D)
	{
		m_D3D->Shutdown();
		delete m_D3D;
		m_D3D = 0;
	}

	return;
}

bool GraphicsClass::Frame()
{
	//render the scene
	bool result;
	result = Render();
	return result;
}

bool GraphicsClass::Render()
{
	//clear buffers at begin
	m_D3D->BeginScene(0.5f, 0.5f, 0.5f, 1.0f);

	//get rendered scene to screen
	m_D3D->EndScene();

	return true;
}