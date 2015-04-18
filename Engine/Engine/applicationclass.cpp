////////////////////////////////////////////////////////////////////////////////
// Filename: applicationclass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "applicationclass.h"


ApplicationClass::ApplicationClass()
{
	currentGameState = INTRO;

	m_Input = 0;
	m_Direct3D = 0;
	m_Camera = 0;
	m_Terrain = 0;

	m_FontShader = 0;
	m_Text = 0;
	m_pointsCollected = 0;

	m_TerrainShader = 0;
	m_SphereShader = 0;
	m_Light = 0;
	m_Frustum = 0;
	m_QuadTree = 0;

	m_DungeonGenerator = 0;
	m_dungeonRecentlyCreated = false;

	m_ColorFilterShader = 0;
	m_RenderTexture = 0;
	m_PostProcessedTexture = 0;
	m_FullScreenWindow = 0;
	
	m_TextureShader = 0;
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

	//--- set up the rendering variables for rednering the loading screen

	//init direct 3D object - handles communication with video card
	m_Direct3D = new D3DClass;
	if (!m_Direct3D)
		return false;

	// Initialize the Direct3D object.
	result = m_Direct3D->Initialize(screenWidth, screenHeight, VSYNC_ENABLED, hwnd, FULL_SCREEN, SCREEN_DEPTH, SCREEN_NEAR);
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize DirectX 11.", L"Error", MB_OK);
		return false;
	}

	//init camera object - handles the 3D rendering (gets position from position class)
	m_Camera = new CameraClass;
	if (!m_Camera)
		return false;

	//get base view matrix to handle the 2D rendering later
	m_Camera->SetPosition(0.0f, 0.0f, -1.0f);
	m_Camera->Render();
	m_Camera->GetViewMatrix(m_baseViewMatrix);

	//init y offset - camera acts as head while the position object is the body -> cmare needs to be over the body
	float cameraOffset = 2.0f;

	//init the full screen ortho window object
	m_FullScreenWindow = new OrthoWindowClass;
	if (!m_FullScreenWindow)
		return false;

	result = m_FullScreenWindow->Initialize(m_Direct3D->GetDevice(), screenWidth, screenHeight);
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the full screen ortho window object.", L"Error", MB_OK);
		return false;
	}

	//init font shader
	m_FontShader = new FontShaderClass;
	if (!m_FontShader)
		return false;

	result = m_FontShader->Initialize(m_Direct3D->GetDevice(), hwnd);
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the font shader object.", L"Error", MB_OK);
		return false;
	}

	//init text object
	m_Text = new TextClass;
	if (!m_Text)
	{
		return false;
	}

	result = m_Text->Initialize(m_Direct3D->GetDevice(), m_Direct3D->GetDeviceContext(), hwnd, screenWidth, screenHeight, m_baseViewMatrix);
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the text object.", L"Error", MB_OK);
		return false;
	}


	//---- render the intro screen to have something on the screen while loading

	// Clear the scene.
	m_Direct3D->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);
	// Turn off the Z buffer to begin all 2D rendering.
	m_Direct3D->TurnZBufferOff();

	//get world and ortho matrix
	D3DXMATRIX worldMatrix, orthoMatrix;
	m_Direct3D->GetWorldMatrix(worldMatrix);
	m_Direct3D->GetOrthoMatrix(orthoMatrix);

	result = m_Text->Render(m_Direct3D->GetDeviceContext(), m_FontShader, worldMatrix, orthoMatrix, (int)currentGameState);
	if (!result)
		return false;

	//turn z buffer back on
	m_Direct3D->TurnZBufferOn();

	// Present the rendered scene to the screen.
	m_Direct3D->EndScene();


	//------ creating the dungeon

	//init dungeon generator - handles all issues with the dungeon itself
	m_DungeonGenerator = new DungeonGeneratorClass;
	if(!m_DungeonGenerator)
		return false;

	result = m_DungeonGenerator->Initialize(DUNGEON_WIDTH, DUNGEON_HEIGHT);
	if(!result)
	{
		MessageBox(hwnd, L"Could not initialize the dungeon object.", L"Error", MB_OK);
		return false;
	}

	//create a new dungeon
	result = m_DungeonGenerator->GenerateNewDungeon(DUNGEON_ROOMS, m_Direct3D);
	if(!result)
	{
		MessageBox(hwnd, L"Could not create a new dungeon.", L"Error", MB_OK);
		return false;
	}


	//----- setting up the rest of the rendering

	//init terrain object
	m_Terrain = new TerrainClass;
	if(!m_Terrain)
		return false;

	result = m_Terrain->Initialize(m_Direct3D->GetDevice(), m_DungeonGenerator->GetDungeonData(), L"../Engine/data/dirt01.dds", L"../Engine/data/rock.dds");
	if(!result)
	{
		MessageBox(hwnd, L"Could not initialize the terrain object.", L"Error", MB_OK);
		return false;
	}

	//init frustum object
	m_Frustum = new FrustumClass;
	if (!m_Frustum)
		return false;

	//init quad tree object
	m_QuadTree = new QuadTreeClass;
	if (!m_QuadTree)
		return false;

	result = m_QuadTree->Initialize(m_Terrain, m_Direct3D->GetDevice(), m_DungeonGenerator->GetDungeonData());
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the quad tree object.", L"Error", MB_OK);
		return false;
	}

	//---- init shaders for the terrain and the collectibles

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


	//----- init shaders for post processing

	//init color filter shader
	m_ColorFilterShader = new ColorFilterShaderClass;
	if (!m_ColorFilterShader)
		return false;

	result = m_ColorFilterShader->Initialize(m_Direct3D->GetDevice(), hwnd);
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the color filter shader object.", L"Error", MB_OK);
		return false;
	}

	//----- init textures for post processing

	//init final texture for post processing
	m_PostProcessedTexture = new RenderTextureClass;
	if(!m_PostProcessedTexture)
		return false;

	result = m_PostProcessedTexture->Initialize(m_Direct3D->GetDevice(), screenWidth, screenHeight, SCREEN_DEPTH, SCREEN_NEAR);
	if(!result)
	{
		MessageBox(hwnd, L"Could not initialize the post processing render to texture object.", L"Error", MB_OK);
		return false;
	}

	//init light object
	m_Light = new LightClass;
	if(!m_Light)
		return false;

	m_Light->SetAmbientColor(0.05f, 0.05f, 0.05f, 1.0f);
	m_Light->SetDiffuseColor(1.0f, 1.0f, 1.0f, 1.0f);
	m_Light->SetDirection(-0.5f, -1.0f, 0.0f);


	//--- set variables for movement and navigation

	//init input object - handles input from keyboard and mouse
	m_Input = new InputClass;
	if (!m_Input)
		return false;

	result = m_Input->Initialize(hinstance, hwnd, screenWidth, screenHeight);
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the input object.", L"Error", MB_OK);
		return false;
	}

	//init timer object -> return frame time 
	m_Timer = new TimerClass;
	if (!m_Timer)
		return false;

	result = m_Timer->Initialize();
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the timer object.", L"Error", MB_OK);
		return false;
	}

	//init position object - handles all physical movement in the 3D world (e.g. collision)
	m_Position = new PositionClass;
	if (!m_Position)
		return false;

	//get spawn position in dungeon
	float startPosX = 0.0f, startPosY = 0.0f, startPosZ = 0.0f;
	m_DungeonGenerator->GetSpawningCoord(startPosX, startPosY, startPosZ);

	// Set the initial position to the spawning position in the dungeon
	m_Position->SetPosition(startPosX, startPosY, startPosZ);
	m_Position->SetCollisionRadius(1.0f);
	m_Position->SetAllowedUpwardDifference(0.1f);

	//set the initial position equal to the one from the position object
	m_Camera->SetPosition(startPosX, startPosY + cameraOffset, startPosZ);
	m_Camera->SetYOffset(cameraOffset);


	//----- init objects for additional information on the screen

	//set text for collected points
	result = m_Text->SetPoints(m_pointsCollected, m_DungeonGenerator->GetTotalPointCount(), m_Direct3D->GetDeviceContext());
	if (!result)
	{
		MessageBox(hwnd, L"Could not display points.", L"Error", MB_OK);
		return false;
	}

	//init render to texture object (target for rendering the complete scene to)
	m_RenderTexture = new RenderTextureClass;
	if (!m_RenderTexture)
		return false;

	result = m_RenderTexture->Initialize(m_Direct3D->GetDevice(), screenWidth, screenHeight, SCREEN_DEPTH, SCREEN_NEAR);
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the render to texture object.", L"Error", MB_OK);
		return false;
	}

	//init texture shader (rendering final texture to screen)
	m_TextureShader = new TextureShaderClass;
	if (!m_TextureShader)
		return false;

	result = m_TextureShader->Initialize(m_Direct3D->GetDevice(), hwnd);
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the texture shader object.", L"Error", MB_OK);
		return false;
	}

	currentGameState = GAME;

	return true;
}

void ApplicationClass::Shutdown()
{
	if(m_FullScreenWindow)
	{
		m_FullScreenWindow->Shutdown();
		delete m_FullScreenWindow;
		m_FullScreenWindow = 0;
	}

	if(m_RenderTexture)
	{
		m_RenderTexture->Shutdown();
		delete m_RenderTexture;
		m_RenderTexture = 0;
	}

	if (m_ColorFilterShader)
	{
		m_ColorFilterShader->Shutdown();
		delete m_ColorFilterShader;
		m_ColorFilterShader = 0;
	}

	if(m_TextureShader)
	{
		m_TextureShader->Shutdown();
		delete m_TextureShader;
		m_TextureShader = 0;
	}

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

	if (m_Timer)
	{
		delete m_Timer;
		m_Timer = 0;
	}

	// Release the position object.
	if (m_Position)
	{
		delete m_Position;
		m_Position = 0;
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
		return false;

	if (currentGameState == GAME)
	{
		//update the frame time
		m_Timer->Frame();

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
		if (!result)
			return false;

		//handle collisions with the wall
		float newX = 0.0f, newY = 0.0f, newZ = 0.0f;
		m_Position->GetPosition(newX, newY, newZ);
		HandleWallCollision(newX, newY, newZ, posX, posY, posZ);
		m_Position->SetPosition(newX, newY, newZ);

		//check if a collectible was collected in this frame
		result = m_DungeonGenerator->CollectibleAtPosition(newX, newY, newZ);
		if (result)
		{
			m_pointsCollected++;
			result = m_Text->SetPoints(m_pointsCollected, m_DungeonGenerator->GetTotalPointCount(), m_Direct3D->GetDeviceContext());
			if (!result)
				return false;
		}

		// Get the view point position/rotation.
		float rotX, rotY, rotZ;
		m_Position->GetRotation(rotX, rotY, rotZ);

		// Set the position of the camera.
		m_Camera->SetPosition(newX, newY + m_Camera->GetYOffset(), newZ);
		m_Camera->SetRotation(rotX, rotY, rotZ);

		// Render the graphics.
		result = RenderGraphics();
		if (!result)
			return false;

		if (m_pointsCollected >= m_DungeonGenerator->GetTotalPointCount())
			currentGameState = END;
	}
	//render end screen
	else
	{
		// Clear the scene.
		m_Direct3D->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);
		// Turn off the Z buffer to begin all 2D rendering.
		m_Direct3D->TurnZBufferOff();

		//get world and ortho matrix
		D3DXMATRIX worldMatrix, orthoMatrix;
		m_Direct3D->GetWorldMatrix(worldMatrix);
		m_Direct3D->GetOrthoMatrix(orthoMatrix);

		bool result = m_Text->Render(m_Direct3D->GetDeviceContext(), m_FontShader, worldMatrix, orthoMatrix, (int)currentGameState);
		if (!result)
			return false;

		//turn z buffer back on
		m_Direct3D->TurnZBufferOn();

		// Present the rendered scene to the screen.
		m_Direct3D->EndScene();

		//handle the input for this frame
		result = HandleInput();
		if (!result)
			return false;
	}
	

	return result;
}

bool ApplicationClass::HandleInput()
{
	bool keyDown;

	//handle the movement and rotation input only when in the game
	if (currentGameState == GAME)
	{
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
		if (keyDown)
			m_Position->Jump();
	}

	//creating a new dungeon
	keyDown = m_Input->IsPPressed();
	if(keyDown && !m_dungeonRecentlyCreated)
	{
		//----- render intro screen to see seomething while loading
		currentGameState = INTRO;
		// Clear the scene.
		m_Direct3D->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);
		// Turn off the Z buffer to begin all 2D rendering.
		m_Direct3D->TurnZBufferOff();

		//get world and ortho matrix
		D3DXMATRIX worldMatrix, orthoMatrix;
		m_Direct3D->GetWorldMatrix(worldMatrix);
		m_Direct3D->GetOrthoMatrix(orthoMatrix);

		bool result = m_Text->Render(m_Direct3D->GetDeviceContext(), m_FontShader, worldMatrix, orthoMatrix, (int)currentGameState);
		if (!result)
			return false;

		//turn z buffer back on
		m_Direct3D->TurnZBufferOn();

		// Present the rendered scene to the screen.
		m_Direct3D->EndScene();


		//------ create new dungeon
		m_DungeonGenerator->GenerateNewDungeon(DUNGEON_ROOMS, m_Direct3D);

		m_Terrain->Shutdown();
		result = m_Terrain->Initialize(m_Direct3D->GetDevice(), m_DungeonGenerator->GetDungeonData(), L"../Engine/data/dirt01.dds", L"../Engine/data/rock.dds");
		if(!result)
			return false;

		m_QuadTree->Shutdown();
		result = m_QuadTree->Initialize(m_Terrain, m_Direct3D->GetDevice(), m_DungeonGenerator->GetDungeonData());
		if(!result)
			return false;

		m_dungeonRecentlyCreated = true;

		//reset position and camera with spawning coordinate from dungeon
		float posX, posY, posZ;
		m_DungeonGenerator->GetSpawningCoord(posX, posY, posZ);
		m_Position->SetPosition(posX, posY, posZ);
		m_Camera->SetPosition(posX, posY + m_Camera->GetYOffset(), posZ);
		m_Camera->SetRotation(0.0f, 0.0f, 0.0f);

		//reset points and game state
		m_pointsCollected = 0;
		currentGameState = GAME;

		//set the text for points back
		result = m_Text->SetPoints(m_pointsCollected, m_DungeonGenerator->GetTotalPointCount(), m_Direct3D->GetDeviceContext());
		if (!result)
			return false;
	}
	else if(!keyDown && m_dungeonRecentlyCreated)
		m_dungeonRecentlyCreated = false;

	return true;
}


bool ApplicationClass::RenderGraphics()
{
	bool result = false;

	//render scene to texture
	result = RenderSceneToTexture();
	if(!result)
		return false;

	result = RenderPostProcessing();
	if(!result)
		return false;

	// Clear the scene.
	m_Direct3D->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

	//m_Camera->Render();
	
	// Turn off the Z buffer to begin all 2D rendering.
	m_Direct3D->TurnZBufferOff();

	//render the scene
	D3DXMATRIX worldMatrix, orthoMatrix;
	m_Direct3D->GetWorldMatrix(worldMatrix);
	//m_Camera->GetViewMatrix(viewMatrix);
	m_Direct3D->GetOrthoMatrix(orthoMatrix);
	//m_RenderTexture->GetOrthoMatrix(orthoMatrix);

	m_FullScreenWindow->Render(m_Direct3D->GetDeviceContext());

	//render the last rendered texture on screen to see the result
	result = m_TextureShader->Render(m_Direct3D->GetDeviceContext(), m_FullScreenWindow->GetIndexCount(),
		worldMatrix, m_baseViewMatrix, orthoMatrix, m_PostProcessedTexture->GetShaderResourceView());

	if(!result)
		return false;

	// Turn on the alpha blending before rendering the text.
	m_Direct3D->TurnOnAlphaBlending();

	// Render the text user interface elements.
	result = m_Text->Render(m_Direct3D->GetDeviceContext(), m_FontShader, worldMatrix, orthoMatrix, (int)currentGameState);
	if(!result)
		return false;
		
	// Turn off alpha blending after rendering the text.
	m_Direct3D->TurnOffAlphaBlending();

	//turn z buffer back on
	m_Direct3D->TurnZBufferOn();

	// Present the rendered scene to the screen.
	m_Direct3D->EndScene();

	return true;
}

bool ApplicationClass::RenderSceneToTexture()
{
	//set render target to render to texture object
	m_RenderTexture->SetRenderTarget(m_Direct3D->GetDeviceContext());

	//clear render to texture
	m_RenderTexture->ClearRenderTarget(m_Direct3D->GetDeviceContext(), 0.0f, 0.0f, 0.0f, 1.0f);

	// Generate the view matrix based on the camera's position.
	m_Camera->Render();
	
	// Get the world, view, projection, and ortho matrices from the camera and Direct3D objects.
	D3DXMATRIX worldMatrix, viewMatrix, projectionMatrix;
	m_Direct3D->GetWorldMatrix(worldMatrix);
	m_Camera->GetViewMatrix(viewMatrix);
	m_Direct3D->GetProjectionMatrix(projectionMatrix);

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

			//set shader parameters for each model individually (because world matrix changed)
			result = m_SphereShader->SetShaderParameters(m_Direct3D->GetDeviceContext(), worldMatrix, viewMatrix, projectionMatrix,
				m_Light->GetAmbientColor(), m_Light->GetDiffuseColor(), m_Light->GetDirection(), currentModel->model->GetTexture());
			if (!result)
				return false;

			currentModel->model->Render(m_Direct3D->GetDeviceContext());
			m_SphereShader->RenderShader(m_Direct3D->GetDeviceContext(), currentModel->model->GetIndexCount());
			if(!result)
				return false;

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
	
	//reset render target to original back buffer and to stop rendering the texture
	m_Direct3D->SetBackBufferRenderTarget();

	//reset viewport back to original
	m_Direct3D->ResetViewport();

	return true;
}

bool ApplicationClass::RenderPostProcessing()
{
	//set post processing texture as target and clean it
	m_PostProcessedTexture->SetRenderTarget(m_Direct3D->GetDeviceContext());
	m_PostProcessedTexture->ClearRenderTarget(m_Direct3D->GetDeviceContext(), 0.0f, 0.0f, 0.0f, 1.0f);

	D3DXMATRIX worldMatrix, orthoMatrix;
	m_Direct3D->GetWorldMatrix(worldMatrix);

	//get orttho mattrix from texture, since the dimensions can differ
	m_PostProcessedTexture->GetOrthoMatrix(orthoMatrix);

	//prepare 2D rendering by turning the z buffer off
	m_Direct3D->TurnZBufferOff();

	//put the index and vertex buffer of the full screen into the graphics pipeline
	m_FullScreenWindow->Render(m_Direct3D->GetDeviceContext());

	//render the full screen window using the post processing shader
	bool result = false;
	result = m_ColorFilterShader->Render(m_Direct3D->GetDeviceContext(), m_FullScreenWindow->GetIndexCount(),
		worldMatrix, m_baseViewMatrix, orthoMatrix, m_RenderTexture->GetShaderResourceView());

	if(!result)
		return false;

	//2D rendering has completed -> turn z buffer on
	m_Direct3D->TurnZBufferOn();

	//reset rendering target to stop rendering the texture
	m_Direct3D->SetBackBufferRenderTarget();

	//reset viewport back to original
	m_Direct3D->ResetViewport();

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