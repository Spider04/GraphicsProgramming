#include "graphicsclass.h"

GraphicsClass::GraphicsClass()
	: m_D3D(0)
	, m_Model(0)
	, m_Camera(0)
	, m_ColorShader(0)
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

	//init camera
	m_Camera = new CameraClass;
	if(!m_Camera)
		return false;

	m_Camera->SetPosition(0.0f, 0.0f, -10.0f);

	//init model
	m_Model = new ModelClass;
	if(!m_Model)
		return false;

	result = m_Model->Initialize(m_D3D->GetDevice());
	if(!result)
	{
		MessageBox(hwnd, L"Could not initialize model object", L"Error", MB_OK);
		return false;
	}

	//init shader object
	m_ColorShader = new ColorShaderClass;
	if(!m_ColorShader)
		return false;

	result = m_ColorShader->Initialize(m_D3D->GetDevice(), hwnd);
	if(!result)
	{
		MessageBox(hwnd, L"Could not initialize color shader object", L"Error", MB_OK);
		return false;
	}

	return true;
}

void GraphicsClass::Shutdown()
{
	if(m_ColorShader)
	{
		m_ColorShader->Shutdown();
		delete m_ColorShader;
		m_ColorShader = 0;
	}

	if(m_Model)
	{
		m_Model->Shutdown();
		delete m_Model;
		m_Model = 0;
	}

	if(m_Camera)
	{
		delete m_Camera;
		m_Camera = 0;
	}

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
	m_D3D->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

	//generate view matrix based on camers's position
	m_Camera->Render();

	//get matrices and D3D objects from camera
	D3DXMATRIX viewMatrix, projectionMatrix, worldMatrix;
	m_Camera->GetViewMatrix(viewMatrix);
	m_D3D->GetWorldMatrix(worldMatrix);
	m_D3D->GetProjectionMatrix(projectionMatrix);
	
	//put model vertex and index buffer in piepeline in order to draw it
	m_Model->Render(m_D3D->GetDeviceContext());

	//render to model with the color shader
	bool result;
	result = m_ColorShader->Render(m_D3D->GetDeviceContext(), m_Model->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix);
	if(!result)
		return false;

	//get rendered scene to screen
	m_D3D->EndScene();

	return true;
}