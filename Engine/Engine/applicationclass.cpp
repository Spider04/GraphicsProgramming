////////////////////////////////////////////////////////////////////////////////
// Filename: applicationclass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "applicationclass.h"


ApplicationClass::ApplicationClass()
{
	m_Input = 0;
	m_Direct3D = 0;
	m_Camera = 0;
	m_Terrain = 0;

	m_Timer = 0;
	m_Position = 0;
	m_Fps = 0;
	m_Cpu = 0;
	m_FontShader = 0;
	m_Text = 0;

	m_TerrainShader = 0;
	m_SphereShader = 0;
	m_Light = 0;
	m_Frustum = 0;
	m_QuadTree = 0;

	m_DungeonGenerator = 0;
	m_dungeonRecentlyCreated = false;
	m_pointsCollected = 0;
}
ApplicationClass::ApplicationClass(const ApplicationClass& other)
{
}

ApplicationClass::~ApplicationClass()
{
}

bool ApplicationClass::Initialize(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight)
{
	bool result = false;

	//create dungeon generator first - influences build of whole level
	m_DungeonGenerator = new DungeonGeneratorClass;
	if(!m_DungeonGenerator)
		return false;

	result = m_DungeonGenerator->Initialize(DUNGEON_WIDTH, DUNGEON_HEIGHT);
	if(!result)
	{
		MessageBox(hwnd, L"Could not initialize the dungeon object.", L"Error", MB_OK);
		return false;
	}

	D3DXMATRIX baseViewMatrix;
	char videoCard[128];
	int videoMemory;

	
	// Create the input object.  The input object will be used to handle reading the keyboard and mouse input from the user.
	m_Input = new InputClass;
	if(!m_Input)
	{
		return false;
	}

	// Initialize the input object.
	result = m_Input->Initialize(hinstance, hwnd, screenWidth, screenHeight);
	if(!result)
	{
		MessageBox(hwnd, L"Could not initialize the input object.", L"Error", MB_OK);
		return false;
	}

	// Create the Direct3D object.
	m_Direct3D = new D3DClass;
	if(!m_Direct3D)
	{
		return false;
	}

	// Initialize the Direct3D object.
	result = m_Direct3D->Initialize(screenWidth, screenHeight, VSYNC_ENABLED, hwnd, FULL_SCREEN, SCREEN_DEPTH, SCREEN_NEAR);
	if(!result)
	{
		MessageBox(hwnd, L"Could not initialize DirectX 11.", L"Error", MB_OK);
		return false;
	}

	//create a new dungeon
	result = m_DungeonGenerator->GenerateNewDungeon(DUNGEON_ROOMS, m_Direct3D);
	if(!result)
	{
		MessageBox(hwnd, L"Could not create a new dungeon.", L"Error", MB_OK);
		return false;
	}

	// Create the camera object.
	m_Camera = new CameraClass;
	if(!m_Camera)
	{
		return false;
	}

	// Initialize a base view matrix with the camera for 2D user interface rendering.
	m_Camera->SetPosition(0.0f, 0.0f, -1.0f);
	m_Camera->Render();
	m_Camera->GetViewMatrix(baseViewMatrix);

	// Set the initial position of the camera.
	float cameraX, cameraY, cameraZ;
	float cameraOffset = 2.0f;
	m_DungeonGenerator->GetSpawningCoord(cameraX, cameraY, cameraZ);
	//cameraX = 50.0f;
	cameraY += cameraOffset;
	//cameraZ = 50.0f;

	m_Camera->SetPosition(cameraX, cameraY, cameraZ);
	m_Camera->SetYOffset(cameraOffset);

	// Create the terrain object.
	m_Terrain = new TerrainClass;
	if(!m_Terrain)
	{
		return false;
	}

	// Initialize the terrain object.
	//result = m_Terrain->Initialize(m_Direct3D->GetDevice(), "../Engine/data/heightmap01.bmp", L"../Engine/data/dirt01.dds");
	result = m_Terrain->Initialize(m_Direct3D->GetDevice(), m_DungeonGenerator->GetDungeonData(), L"../Engine/data/dirt01.dds", L"../Engine/data/rock.dds", L"../Engine/data/red.dds");
	if(!result)
	{
		MessageBox(hwnd, L"Could not initialize the terrain object.", L"Error", MB_OK);
		return false;
	}

	// Create the timer object.
	m_Timer = new TimerClass;
	if(!m_Timer)
	{
		return false;
	}

	// Initialize the timer object.
	result = m_Timer->Initialize();
	if(!result)
	{
		MessageBox(hwnd, L"Could not initialize the timer object.", L"Error", MB_OK);
		return false;
	}

	// Create the position object.
	m_Position = new PositionClass;
	if(!m_Position)
	{
		return false;
	}

	// Set the initial position of the viewer to the same as the initial camera position.
	m_Position->SetPosition(cameraX, cameraY - m_Camera->GetYOffset(), cameraZ);
	m_Position->SetCollisionRadius(1.0f);
	m_Position->SetAllowedUpwardDifference(0.1f);

	// Create the fps object.
	m_Fps = new FpsClass;
	if(!m_Fps)
	{
		return false;
	}

	// Initialize the fps object.
	m_Fps->Initialize();

	// Create the cpu object.
	m_Cpu = new CpuClass;
	if(!m_Cpu)
	{
		return false;
	}

	// Initialize the cpu object.
	m_Cpu->Initialize();

	// Create the font shader object.
	m_FontShader = new FontShaderClass;
	if(!m_FontShader)
	{
		return false;
	}

	// Initialize the font shader object.
	result = m_FontShader->Initialize(m_Direct3D->GetDevice(), hwnd);
	if(!result)
	{
		MessageBox(hwnd, L"Could not initialize the font shader object.", L"Error", MB_OK);
		return false;
	}

	// Create the text object.
	m_Text = new TextClass;
	if(!m_Text)
	{
		return false;
	}

	// Initialize the text object.
	result = m_Text->Initialize(m_Direct3D->GetDevice(), m_Direct3D->GetDeviceContext(), hwnd, screenWidth, screenHeight, baseViewMatrix);
	if(!result)
	{
		MessageBox(hwnd, L"Could not initialize the text object.", L"Error", MB_OK);
		return false;
	}

	// Retrieve the video card information.
	m_Direct3D->GetVideoCardInfo(videoCard, videoMemory);

	// Set the video card information in the text object.
	result = m_Text->SetVideoCardInfo(videoCard, videoMemory, m_Direct3D->GetDeviceContext());
	if(!result)
	{
		MessageBox(hwnd, L"Could not set video card info in the text object.", L"Error", MB_OK);
		return false;
	}


	//init terrain shader
	m_TerrainShader = new TerrainShaderClass;
	if(!m_TerrainShader)
		return false;

	result = m_TerrainShader->Initialize(m_Direct3D->GetDevice(), hwnd);
	if(!result)
	{
		MessageBox(hwnd, L"Could not initialize the terrain shader object.", L"Error", MB_OK);
		return false;
	}

	//init sphere shader
	m_SphereShader = new SphereShaderClass;
	if(!m_SphereShader)
		return false;

	result = m_SphereShader->Initialize(m_Direct3D->GetDevice(), hwnd);
	if(!result)
	{
		MessageBox(hwnd, L"Could not initialize the sphere shader object.", L"Error", MB_OK);
		return false;
	}


	//init light object
	m_Light = new LightClass;
	if(!m_Light)
		return false;

	m_Light->SetAmbientColor(0.05f, 0.05f, 0.05f, 1.0f);
	m_Light->SetDiffuseColor(1.0f, 1.0f, 1.0f, 1.0f);
	m_Light->SetDirection(-0.5f, -1.0f, 0.0f);

	//init frustum object
	m_Frustum = new FrustumClass;
	if(!m_Frustum)
		return false;

	//init quad tree object
	m_QuadTree = new QuadTreeClass;
	if(!m_QuadTree)
		return false;

	result = m_QuadTree->Initialize(m_Terrain, m_Direct3D->GetDevice(), m_DungeonGenerator->GetDungeonData());
	if(!result)
	{
		MessageBox(hwnd, L"Could not initialize the quad tree object.", L"Error", MB_OK);
		return false;
	}

	
	//set seed text for this dungeon
	result = m_Text->SetDungeonRandSeed(m_DungeonGenerator->GetDungeonSeed(), m_Direct3D->GetDeviceContext());
	if(!result)
	{
		MessageBox(hwnd, L"Could not get dungeon seed.", L"Error", MB_OK);
		return false;
	}

	//set point text
	result = m_Text->SetPoints(m_pointsCollected, m_DungeonGenerator->GetTotalPointCount(), m_Direct3D->GetDeviceContext());
	if(!result)
	{
		MessageBox(hwnd, L"Could not display points.", L"Error", MB_OK);
		return false;
	}

	return true;
}

void ApplicationClass::Shutdown()
{
	if(m_DungeonGenerator)
	{
		m_DungeonGenerator->Shutdown();
		delete m_DungeonGenerator;
		m_DungeonGenerator = 0;
	}

	if(m_QuadTree)
	{
		m_QuadTree->Shutdown();
		delete m_QuadTree;
		m_QuadTree = 0;
	}

	if(m_Frustum)
	{
		delete m_Frustum;
		m_Frustum = 0;
	}

	if(m_Light)
	{
		delete m_Light;
		m_Light = 0;
	}

	if(m_TerrainShader)
	{
		m_TerrainShader->Shutdown();
		delete m_TerrainShader;
		m_TerrainShader = 0;
	}

	if(m_SphereShader)
	{
		m_SphereShader->Shutdown();
		delete m_SphereShader;
		m_SphereShader = 0;
	}

	// Release the text object.
	if(m_Text)
	{
		m_Text->Shutdown();
		delete m_Text;
		m_Text = 0;
	}

	// Release the font shader object.
	if(m_FontShader)
	{
		m_FontShader->Shutdown();
		delete m_FontShader;
		m_FontShader = 0;
	}

	// Release the cpu object.
	if(m_Cpu)
	{
		m_Cpu->Shutdown();
		delete m_Cpu;
		m_Cpu = 0;
	}

	// Release the fps object.
	if(m_Fps)
	{
		delete m_Fps;
		m_Fps = 0;
	}

	// Release the position object.
	if(m_Position)
	{
		delete m_Position;
		m_Position = 0;
	}

	// Release the timer object.
	if(m_Timer)
	{
		delete m_Timer;
		m_Timer = 0;
	}

	// Release the terrain object.
	if(m_Terrain)
	{
		m_Terrain->Shutdown();
		delete m_Terrain;
		m_Terrain = 0;
	}

	// Release the camera object.
	if(m_Camera)
	{
		delete m_Camera;
		m_Camera = 0;
	}

	// Release the Direct3D object.
	if(m_Direct3D)
	{
		m_Direct3D->Shutdown();
		delete m_Direct3D;
		m_Direct3D = 0;
	}

	// Release the input object.
	if(m_Input)
	{
		m_Input->Shutdown();
		delete m_Input;
		m_Input = 0;
	}

	return;
}


bool ApplicationClass::Frame()
{
	bool result;

	// Read the user input.
	result = m_Input->Frame();
	if(!result)
	{
		return false;
	}
	
	// Check if the user pressed escape and wants to exit the application.
	if(m_Input->IsEscapePressed() == true)
	{
		return false;
	}

	// Update the system stats.
	m_Timer->Frame();
	m_Fps->Frame();
	m_Cpu->Frame();

	// Update the FPS value in the text object.
	result = m_Text->SetFps(m_Fps->GetFps(), m_Direct3D->GetDeviceContext());
	if(!result)
	{
		return false;
	}
	
	// Update the CPU usage value in the text object.
	result = m_Text->SetCpu(m_Cpu->GetCpuPercentage(), m_Direct3D->GetDeviceContext());
	if(!result)
	{
		return false;
	}

	//record old position
	float posX, posY, posZ;
	m_Position->GetPosition(posX, posY, posZ);

	bool foundHeight = false;
	foundHeight = m_QuadTree->GetHeightAtPosition(posX, posZ, posY);
	
	// Set the frame time for calculating the updated position.
	// and calculate physics
	m_Position->Frame(m_Timer->GetTime(), posY, foundHeight);

	//record position after physics are calulated
	m_Position->GetPosition(posX, posY, posZ);

	// Do the frame input processing.
	result = HandleInput();
	if(!result)
		return false;

	//handle collisions with the wall
	float newX = 0.0f, newY = 0.0f, newZ = 0.0f;
	m_Position->GetPosition(newX, newY, newZ);
	HandleWallCollision(newX, newY, newZ, posX, posY, posZ);
	m_Position->SetPosition(newX, newY, newZ);
	
	//check if a collectible was collected in this frame
	result = m_DungeonGenerator->CollectibleAtPosition(newX, newY, newZ);
	if(result)
	{
		m_pointsCollected++;
		result = m_Text->SetPoints(m_pointsCollected, m_DungeonGenerator->GetTotalPointCount(), m_Direct3D->GetDeviceContext());
		if(!result)
			return false;
	}
	
	// Get the view point position/rotation.
	float rotX, rotY, rotZ;
	m_Position->GetRotation(rotX, rotY, rotZ);

	// Set the position of the camera.
	m_Camera->SetPosition(newX, newY + m_Camera->GetYOffset(), newZ);
	m_Camera->SetRotation(rotX, rotY, rotZ);

	// Update the position values in the text object.
	result = m_Text->SetCameraPosition(posX, posY, posZ, m_Direct3D->GetDeviceContext());
	if(!result)
		return false;

	// Update the rotation values in the text object.
	result = m_Text->SetCameraRotation(rotX, rotY, rotZ, m_Direct3D->GetDeviceContext());
	if(!result)
		return false;

	// Render the graphics.
	result = RenderGraphics();
	if(!result)
		return false;

	return result;
}

bool ApplicationClass::HandleInput()
{
	bool keyDown;

	// Handle the input.
	keyDown = m_Input->IsUpPressed();
	m_Position->MoveForward(keyDown);
	
	keyDown = m_Input->IsRightPressed();
	m_Position->TurnRight(keyDown);
	
	keyDown = m_Input->IsDownPressed();
	m_Position->MoveBackward(keyDown);
	
	keyDown = m_Input->IsLeftPressed();
	m_Position->TurnLeft(keyDown);

	keyDown = m_Input->IsAPressed();
	m_Position->MoveUpward(keyDown);

	keyDown = m_Input->IsZPressed();
	m_Position->MoveDownward(keyDown);

	keyDown = m_Input->IsPgUpPressed();
	m_Position->LookUpward(keyDown);

	keyDown = m_Input->IsPgDownPressed();
	m_Position->LookDownward(keyDown);

	keyDown = m_Input->IsSpacebarPressed();
	if(keyDown)
		m_Position->Jump();

	//creating a new dungeon
	keyDown = m_Input->IsPPressed();
	if(keyDown && !m_dungeonRecentlyCreated)
	{
		//create new dungeon
		m_DungeonGenerator->GenerateNewDungeon(DUNGEON_ROOMS, m_Direct3D);

		m_Terrain->Shutdown();
		bool result = m_Terrain->Initialize(m_Direct3D->GetDevice(), m_DungeonGenerator->GetDungeonData(), L"../Engine/data/dirt01.dds", L"../Engine/data/rock.dds", L"../Engine/data/red.dds");
		if(!result)
			return false;

		m_QuadTree->Shutdown();
		result = m_QuadTree->Initialize(m_Terrain, m_Direct3D->GetDevice(), m_DungeonGenerator->GetDungeonData());
		if(!result)
			return false;

		m_dungeonRecentlyCreated = true;

		//set seed text for this dungeon
		result = m_Text->SetDungeonRandSeed(m_DungeonGenerator->GetDungeonSeed(), m_Direct3D->GetDeviceContext());
		if(!result)
			return false;

		//reset position and camera with spawning coordinate from dungeon
		float posX, posY, posZ;
		m_DungeonGenerator->GetSpawningCoord(posX, posY, posZ);
		m_Position->SetPosition(posX, posY, posZ);
		m_Camera->SetPosition(posX, posY + m_Camera->GetYOffset(), posZ);
		m_Camera->SetRotation(0.0f, 0.0f, 0.0f);

		//reset points
		m_pointsCollected = 0;
	}
	else if(!keyDown && m_dungeonRecentlyCreated)
		m_dungeonRecentlyCreated = false;

	return true;
}

bool ApplicationClass::RenderGraphics()
{
	// Clear the scene.
	m_Direct3D->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

	// Generate the view matrix based on the camera's position.
	m_Camera->Render();

	// Get the world, view, projection, and ortho matrices from the camera and Direct3D objects.
	D3DXMATRIX worldMatrix, viewMatrix, projectionMatrix, orthoMatrix;
	m_Direct3D->GetWorldMatrix(worldMatrix);
	m_Camera->GetViewMatrix(viewMatrix);
	m_Direct3D->GetProjectionMatrix(projectionMatrix);
	m_Direct3D->GetOrthoMatrix(orthoMatrix);

	bool result = false;

	//render all collectibles (for now, regardless of visibility)
	std::deque<DungeonGeneratorClass::CollectibleData*> *modelList = m_DungeonGenerator->GetDungeonData()->collectibles;
	DungeonGeneratorClass::CollectibleData* currentModel;
	float posX, posY, posZ;

	if(modelList)
	{
		modelList->push_back(0);
		currentModel = modelList->front();

		modelList->pop_front();
		m_Direct3D->GetWorldMatrix(worldMatrix);

		while(currentModel)
		{
			currentModel->model->GetPosition(posX, posY, posZ);
			D3DXMatrixTranslation(&worldMatrix, posX, posY, posZ);

			result = m_SphereShader->SetShaderParameters(m_Direct3D->GetDeviceContext(), worldMatrix, viewMatrix, projectionMatrix,
				m_Light->GetAmbientColor(), m_Light->GetDiffuseColor(), m_Light->GetDirection(), m_Terrain->GetSphereTexture());

			if(!result)
				return false;

			currentModel->model->Render(m_Direct3D->GetDeviceContext());
			m_SphereShader->RenderShader(m_Direct3D->GetDeviceContext(), currentModel->model->GetIndexCount());

			modelList->push_back(currentModel);
			currentModel = modelList->front();
			modelList->pop_front();
		}
	}

	D3DXMatrixTranslation(&worldMatrix, 0.0f, 0.0f, 0.0f);

	//construct frustum based on view and projection matrix
	m_Frustum->ConstructFrustum(SCREEN_DEPTH, projectionMatrix, viewMatrix);

	//set terrain shader parameters before rendering the nodes
	result = m_TerrainShader->SetShaderParameters(m_Direct3D->GetDeviceContext(), worldMatrix, viewMatrix, projectionMatrix,
		m_Light->GetAmbientColor(), m_Light->GetDiffuseColor(), m_Light->GetDirection(), m_Terrain->GetFloorTexture(), m_Terrain->GetWallTexture());

	if(!result)
		return false;

	//render all visible nodes (frustum object for culling, terrain shader for drawing
	m_QuadTree->Render(m_Frustum, m_Direct3D->GetDeviceContext(), m_TerrainShader);

	//set number of drawn triangles
	result = m_Text->SetRenderCount(m_QuadTree->GetDrawCount(), m_Direct3D->GetDeviceContext());
	if(!result)
		return false;


	// Turn off the Z buffer to begin all 2D rendering.
	m_Direct3D->TurnZBufferOff();
		
	// Turn on the alpha blending before rendering the text.
	m_Direct3D->TurnOnAlphaBlending();

	// Render the text user interface elements.
	result = m_Text->Render(m_Direct3D->GetDeviceContext(), m_FontShader, worldMatrix, orthoMatrix);
	if(!result)
		return false;

	// Turn off alpha blending after rendering the text.
	m_Direct3D->TurnOffAlphaBlending();

	// Turn the Z buffer back on now that all 2D rendering has completed.
	m_Direct3D->TurnZBufferOn();

	// Present the rendered scene to the screen.
	m_Direct3D->EndScene();

	return true;
}

void ApplicationClass::HandleWallCollision(float& posX, float& posY, float& posZ, float oldX, float oldY, float oldZ)
{
	float collisionRadius = 0.0f;
	m_Position->GetCollisionRadius(collisionRadius);
	float allowedUpwardsDifference = 0.0f;
	m_Position->GetAllowedUpwardDifference(allowedUpwardsDifference);

	//check if position has clipped with wall - check this with the collision radius in all 4 corners
	bool clipWithWall = false;
	for(int i = 0; i < 4; i++)
	{
		float tmpX = posX;
		float tmpZ = posZ;
		float height = 0.0f;

		if(i == 0)
			tmpX += collisionRadius;
		else if(i == 1)
			tmpX -= collisionRadius;
		else if(i == 2)
			tmpZ += collisionRadius;
		else
			tmpZ -= collisionRadius;

		bool foundHeight = m_QuadTree->GetHeightAtPosition(tmpX, tmpZ, height);
		if(foundHeight)
		{
			if(height > (oldY + allowedUpwardsDifference))
				clipWithWall = true;
		}
		//outside of mesh = world wall
		else
			clipWithWall = true;

		if(clipWithWall)
			break;
	}

	//in case of collision with wall -> set position back to the old one
	if(clipWithWall){
		posX = oldX;
		posY = oldY;
		posZ = oldZ;
		//m_Position->SetPosition(x, y, z);
	}
}