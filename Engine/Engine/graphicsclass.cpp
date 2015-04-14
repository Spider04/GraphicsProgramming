#include "graphicsclass.h"

GraphicsClass::GraphicsClass()
	: m_D3D(0)
	, m_Model(0)
	, m_Camera(0)
	, m_LightShader(0)
	, m_Light(0)
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

	result = m_Model->Initialize(m_D3D->GetDevice(), L"../Engine/data/seafloor.dds");
	if(!result)
	{
		MessageBox(hwnd, L"Could not initialize model object", L"Error", MB_OK);
		return false;
	}

	//init light shader object
	m_LightShader = new LightShaderClass;
	if(!m_LightShader)
		return false;

	result = m_LightShader->Initialize(m_D3D->GetDevice(), hwnd);
	if(!result)
	{
		MessageBox(hwnd, L"Could not initialize light shader object", L"Error", MB_OK);
		return false;
	}

	//init light object
	m_Light = new LightClass;
	if(!m_Light)
		return false;

	m_Light->SetDiffuseColor(1.0f, 0.0f, 1.0f, 1.0f);
	m_Light->SetDirection(0.0f, 0.0f, 1.0f);

	return true;
}

void GraphicsClass::Shutdown()
{
	
	if(m_Light)
	{
		delete m_Light;
		m_Light = 0;
	}

	if(m_LightShader)
	{
		m_LightShader->Shutdown();
		delete m_LightShader;
		m_LightShader = 0;
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
	static float rotation = 0.0f;

	//update roation each frame
	rotation += (float)D3DX_PI * 0.01f;
	if(rotation > 360.0f)
		rotation -= 360.0f;

	//render the scene
	bool result;
	result = Render(rotation);
	return result;
}

bool GraphicsClass::Render(float rotation)
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

	//rotate world matrix -> object will spin
	D3DXMatrixRotationY(&worldMatrix, rotation);
	
	//put model vertex and index buffer in piepeline in order to draw it
	m_Model->Render(m_D3D->GetDeviceContext());

	//render to model with the color shader
	bool result;
	result = m_LightShader->Render(m_D3D->GetDeviceContext(), m_Model->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix,
		m_Model->GetTexture(), m_Light->GetDirection(), m_Light->GetDiffuseColor());
	if(!result)
		return false;

	//get rendered scene to screen
	m_D3D->EndScene();

	return true;
}